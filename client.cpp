#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include <json/json.h>
#include <condition_variable>
#include <mutex>

typedef websocketpp::client<websocketpp::config::asio_client> client;

enum Player {
    NONE = 0,
    CLIENT = 1,
    SERVER = 2
};

struct Configuration {
    std::mutex connection_mutex;
    std::condition_variable connection_cv;
    bool connection_open = false;
    bool my_turn = false;
    bool game_over = false;
    std::string player_name;
};

// Create a global instance of Configuration
Configuration config;

/**
 * @brief Callback function for when a new connection is opened.
 * @param c The client instance.
 * @param hdl The connection handle.
 */
void on_open(client* c, websocketpp::connection_hdl hdl) {
    std::cout << "New connection. Waiting for player name..." << std::endl << std::flush;

    std::lock_guard<std::mutex> lock(config.connection_mutex);
    config.connection_open = true;
    config.connection_cv.notify_one();

    std::cout << "Connected to server. Enter your name: ";
    std::getline(std::cin, config.player_name);

    // Send player name to server
    Json::Value name_message;
    name_message["type"] = "player_name";
    name_message["name"] = config.player_name;

    Json::StreamWriterBuilder writer;
    std::string name_str = Json::writeString(writer, name_message);
    c->send(hdl, name_str, websocketpp::frame::opcode::text);
}

/**
 * @brief Callback function for when the connection is closed.
 * @param c The client instance.
 * @param hdl The connection handle.
 */
void on_close(client* c, websocketpp::connection_hdl hdl) {
    std::cout << "Lost connection to server" << std::endl;
    config.game_over = true;
    config.connection_open = false;
}

/**
 * @brief Prints the game board.
 * @param boardJson The JSON representation of the game board.
 */
void print_board(const Json::Value& boardJson) {
    for (const auto& row : boardJson) {
        for (const auto& cell : row) {
            std::cout << (cell.asInt() == Player::NONE ? "." : (cell.asInt() == Player::CLIENT ? "X" : "O")) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/**
 * @brief Sends a move to the server.
 * @param c The client instance.
 * @param hdl The connection handle.
 * @param column The column to drop the disc.
 */
void send_move(client* c, websocketpp::connection_hdl hdl, std::string column) {
    std::cout << "send_move: " << column << std::endl;
    Json::Value move;
    move["type"] = "move";
    move["column"] = column;

    Json::StreamWriterBuilder writer;
    std::string move_str = Json::writeString(writer, move);
    c->send(hdl, move_str, websocketpp::frame::opcode::text);
}

/**
 * @brief Handles the game start event.
 */
void handle_game_start() {
    std::cout << "The game has started. You are playing as 'X'." << std::endl;
    std::cout << "Waiting for server to make a move..." << std::endl;
}

/**
 * @brief Handles the player's turn event.
 * @param c The client instance.
 * @param hdl The connection handle.
 */
void handle_your_turn(client* c, websocketpp::connection_hdl hdl) {
    config.my_turn = true;
    std::cout << "It's your turn!" << std::endl;

    std::string column;
    std::cout << "Enter column (0-6) to drop your disc: ";
    std::cin >> column;
    send_move(c, hdl, column);

    std::cout << "Waiting for server to make a move..." << std::endl;
    config.my_turn = false;
}

/**
 * @brief Handles the move result event.
 * @param c The client instance.
 * @param hdl The connection handle.
 * @param root The JSON representation of the move result.
 */
void handle_move_result(client* c, websocketpp::connection_hdl hdl, const Json::Value& root) {
    if (root.isMember("error")) {
        std::cout << "Error: " << root["error"].asString() << std::endl;
        config.my_turn = true;
        handle_your_turn(c, hdl);
        return;
    }

    if (root.isMember("board")) {
        print_board(root["board"]);
    }

    bool win = root["win"].asBool();
    if (win) {
        if (root["winner"].asInt() == Player::CLIENT) {
            std::cout << "You won the game!" << std::endl;
        } else {
            std::cout << "You lost the game!" << std::endl;
        }
        config.game_over = true;
    }
}

/**
 * @brief Callback function for when a message is received.
 * @param c The client instance.
 * @param hdl The connection handle.
 * @param msg The message received.
 */
void on_message(client* c, websocketpp::connection_hdl hdl, client::message_ptr msg) {
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream stream(msg->get_payload());
    if (!Json::parseFromStream(reader, stream, &root, &errs)) {
        std::cerr << "Failed to parse message: " << errs << std::endl;
        return;
    }

    std::string message_type = root["type"].asString();
    if (message_type == "game_start") {
        handle_game_start();
    } else if (message_type == "move_result") {
        handle_move_result(c, hdl, root);
    } else if (message_type == "your_turn") {
        handle_your_turn(c, hdl);
    }
}

/**
 * @brief The main function.
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 * @return The exit status.
 */
int main(int argc, char* argv[]) {
    client ws_client;

    // Clear all access channels to suppress detailed frame information
    ws_client.clear_access_channels(websocketpp::log::alevel::all);

    // Set only the application-level logging
    ws_client.set_access_channels(websocketpp::log::alevel::app);

    // Clear specific error channels to suppress transport errors
    ws_client.clear_error_channels(websocketpp::log::elevel::all);

    ws_client.set_message_handler(bind(&on_message, &ws_client, std::placeholders::_1, std::placeholders::_2));
    ws_client.set_open_handler(bind(&on_open, &ws_client, std::placeholders::_1));
    ws_client.set_close_handler(bind(&on_close, &ws_client, std::placeholders::_1));

    ws_client.init_asio();
    websocketpp::lib::error_code ec;
    client::connection_ptr con = ws_client.get_connection("ws://localhost:9002", ec);

    if (ec) {
        std::cerr << "Could not create connection because: " << ec.message() << std::endl;
        return 1; // Exit if connection cannot be established
    }

    std::cout << "Looking for server..." << std::endl;

    ws_client.connect(con);

    // Run the client in a separate thread
    std::thread client_thread([&ws_client]() {
        ws_client.run();
    });

    // Wait for the connection to be established
    {
        std::unique_lock<std::mutex> lock(config.connection_mutex);
        config.connection_cv.wait(lock, [] { return config.connection_open; });
    }

    client_thread.join();
    return 0;
}