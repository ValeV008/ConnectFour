#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include "ConnectFourGame.h"
#include "DatabaseManager.h"
#include <iostream>
#include <string>
#include <functional>
#include <json/json.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "server.h"

typedef websocketpp::server<websocketpp::config::asio> server;

/**
 * @brief Gets the instance of the ConnectFourServer.
 * @return The instance of the ConnectFourServer.
 */
ConnectFourServer& ConnectFourServer::getInstance() {
    static ConnectFourServer instance;
    return instance;
}

/**
 * @brief Constructor for the ConnectFourServer class.
 */
ConnectFourServer::ConnectFourServer() : current_player(Player::SERVER), client_connected(false), game_over(false) {}

/**
 * @brief Destructor for the ConnectFourServer class.
 */
ConnectFourServer::~ConnectFourServer() {}


/**
 * @brief Runs the server and begins listening for connections.
 */
void ConnectFourServer::run() {
    ws_server.clear_access_channels(websocketpp::log::alevel::all);
    ws_server.set_access_channels(websocketpp::log::alevel::app);
    ws_server.clear_error_channels(websocketpp::log::elevel::all);

    ws_server.set_open_handler(bind(&ConnectFourServer::on_open, this, std::placeholders::_1));
    ws_server.set_close_handler(bind(&ConnectFourServer::on_close, this, std::placeholders::_1));
    ws_server.set_message_handler(bind(&ConnectFourServer::on_message, this, std::placeholders::_1, std::placeholders::_2));

    ws_server.init_asio();
    ws_server.listen(9002);
    ws_server.start_accept();

    std::thread server_thread([this]() {
        ws_server.run();
    });

    {
        std::unique_lock<std::mutex> lock(connection_mutex);
        connection_cv.wait(lock, [this] { return client_connected; });
    }

    server_thread.join();
}

/**
 * @brief Sends a JSON message to a specific client.
 * @param hdl The connection handle of the recipient.
 * @param message The JSON message to send.
 */
void ConnectFourServer::send_json_message(websocketpp::connection_hdl hdl, const Json::Value& message) {
    std::string message_str = Json::writeString(Json::StreamWriterBuilder(), message);
    ws_server.send(hdl, message_str, websocketpp::frame::opcode::text);
}

/**
 * @brief Makes a move for the server.
 */
void ConnectFourServer::make_server_move() {
    int server_column;
    std::string input;

    while (true) {
        std::cout << "It's your turn! Enter column (0-6) to drop your disc: ";
        std::cin >> input;

        try {
            server_column = std::stoi(input);
            if (!game.make_move(Player::SERVER, server_column)) {
                throw std::runtime_error("Column is full or out of bounds. Please try a different column.");
            }
            break;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid column value!" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    game.print_board();
    bool win = game.check_winner(Player::SERVER);

    Json::Value response;
    response["type"] = "move_result";
    response["win"] = win;
    response["winner"] = win ? Player::SERVER : Player::NONE;
    response["board"] = game.get_board_json();
    send_json_message(client_hdl, response);

    if (win) {
        std::cout << "Server wins!" << std::endl;
        db_manager.update_or_insert_player_elo(player_name, -1);
        game_over = true;
        return;
    }

    current_player = Player::CLIENT;
    Json::Value turn_notification;
    turn_notification["type"] = "your_turn";
    send_json_message(client_hdl, turn_notification);
    std::cout << "Waiting for client to make a move..." << std::endl;
}

/**
 * @brief Handles a new WebSocket connection.
 * @param hdl The connection handle.
 */
void ConnectFourServer::on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connection_mutex);

    if (client_connected) {
        std::cout << "A client is already connected. Closing new connection." << std::endl;
        ws_server.close(hdl, websocketpp::close::status::normal, "Another client is already connected.");
        return;
    }

    std::cout << "New client connected. Waiting for player name..." << std::endl << std::flush;

    client_connected = true;
    client_hdl = hdl;
    connection_cv.notify_one();

    current_player = Player::SERVER;
    game = ConnectFourGame();
    game_over = false;
}

/**
 * @brief Handles a WebSocket connection closure.
 * @param hdl The connection handle.
 */
void ConnectFourServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connection_mutex);

    auto current_client = client_hdl.lock();
    auto closing_client = hdl.lock();

    if (current_client && closing_client && current_client == closing_client) {
        client_connected = false;

        if (!game_over) {
            std::cout << "Client disconnected. Treating as a loss for the client." << std::endl;
            db_manager.update_or_insert_player_elo(player_name, -1);
            game_over = true;
        }
    } else {
        std::cout << "Rejected connection closed." << std::endl;
    }
}

/**
 * @brief Handles incoming messages from clients.
 * @param hdl The connection handle.
 * @param msg The received message.
 */
void ConnectFourServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::lock_guard<std::mutex> lock(connection_mutex);
    if (!client_connected) {
        std::cerr << "Received message after client disconnected. Ignoring message." << std::endl;
        return;
    }

    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream stream(msg->get_payload());
    if (!Json::parseFromStream(reader, stream, &root, &errs)) {
        std::cerr << "Failed to parse message: " << errs << std::endl;
        return;
    }

    std::string message_type = root["type"].asString();
    if (message_type == "player_name") {
        handle_player_name(root["name"].asString());
    } else if (message_type == "move" && current_player == Player::CLIENT) {
        handle_client_move(root["column"].asString());
    }
}

/**
 * @brief Handles a player's name.
 * @param player_name The name of the player.
 */
void ConnectFourServer::handle_player_name(const std::string& player_name) {
    this->player_name = player_name;
    std::cout << "Player name received: " << player_name << std::endl;

    int elo = db_manager.get_player_elo(player_name);
    if (elo != -1) {
        std::cout << "Player " << player_name << " has an ELO of " << elo << "." << std::endl;
    } else {
        std::cout << "Player " << player_name << " is new. Starting ELO is 100." << std::endl;
    }

    make_server_move();
}


/**
 * @brief Handles a client move.
 * @param column The column to place the piece.
 */
void ConnectFourServer::handle_client_move(const std::string& column) {
    int client_column;
    Json::Value response;
    response["type"] = "move_result";

    try {
        client_column = std::stoi(column);
        if (!game.make_move(Player::CLIENT, client_column)) {
            throw std::runtime_error("Invalid move. Please try a different column.");
        }
    } catch (std::exception& e) {
        std::cerr << "Invalid client move: " << column << std::endl;
        response["error"] = e.what();
        send_json_message(client_hdl, response);
        return;
    }

    game.print_board();
    bool win = game.check_winner(Player::CLIENT);

    response["win"] = win;
    response["winner"] = win ? Player::CLIENT : Player::NONE;
    response["board"] = game.get_board_json();
    send_json_message(client_hdl, response);

    if (win) {
        std::cout << "Client wins!" << std::endl;
        db_manager.update_or_insert_player_elo(player_name, 1);
        game_over = true;
        return;
    }

    current_player = Player::SERVER;
    make_server_move();
}

/**
 * @brief Main function to start the game server.
 * @return Exit status of the program.
 */
int main() {
    ConnectFourServer& server = ConnectFourServer::getInstance();
    server.run();
    return 0;
}