#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <string>
#include <json/json.h>
#include <condition_variable>
#include <mutex>
#include <random>

typedef websocketpp::client<websocketpp::config::asio_client> client;

/**
 * @enum Player
 * @brief Represents the players in the game.
 */
enum Player {
    NONE = 0,   /**< No player */
    CLIENT = 1, /**< The client player */
    SERVER = 2  /**< The server player */
};

std::mutex connection_mutex;
std::condition_variable connection_cv;
bool connection_open = false;
bool my_turn = false; // Start with false to wait for server confirmation
bool game_over = false; // Flag to indicate if the game is over

/**
 * @brief Callback function for when a new connection is opened.
 * @param c The client instance.
 * @param hdl The connection handle.
 */
void on_open(client* c, websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connection_mutex);
    connection_open = true;
    connection_cv.notify_one();

    // Send bot name to server
    Json::Value name_message;
    name_message["type"] = "player_name";
    name_message["name"] = "random Luka";

    Json::StreamWriterBuilder writer;
    std::string name_str = Json::writeString(writer, name_message);
    c->send(hdl, name_str, websocketpp::frame::opcode::text);
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
 * @brief Callback function for when a message is received.
 * @param c The client instance.
 * @param hdl The connection handle.
 * @param msg The message received.
 */
void on_message(client* c, websocketpp::connection_hdl hdl, client::message_ptr msg) {
    // Parse the server's response
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream stream(msg->get_payload());
    if (!Json::parseFromStream(reader, stream, &root, &errs)) {
        std::cerr << "Failed to parse message: " << errs << std::endl;
        return;
    }

    // Handle the response
    if (root["type"].asString() == "game_start") {
        std::cout << "The game has started. You are playing as 'X'." << std::endl;
        std::cout << "Waiting for server to make a move..." << std::endl;
    } else if (root["type"].asString() == "move_result") {
        if (root.isMember("error")) {
            std::cout << "Error: " << root["error"].asString() << std::endl;
            my_turn = true; // Allow the client to retry the move
            return;
        }

        // Print the board
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
            game_over = true; // Set the game over flag
        }
    } else if (root["type"].asString() == "your_turn") {
        my_turn = true; // Server indicates it's the client's turn
        std::cout << "It's your turn!" << std::endl;
    }
}

/**
 * @brief Sends a move to the server.
 * @param c The client instance.
 * @param hdl The connection handle.
 * @param player The player making the move.
 * @param column The column to drop the disc.
 */
void send_move(client* c, websocketpp::connection_hdl hdl, Player player, int column) {
    Json::Value move;
    move["type"] = "move";
    move["player"] = player;
    move["column"] = column;

    Json::StreamWriterBuilder writer;
    std::string move_str = Json::writeString(writer, move);
    c->send(hdl, move_str, websocketpp::frame::opcode::text);
}

/**
 * @brief Main function to run the random Luka bot.
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 * @return Exit status of the program.
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

    ws_client.init_asio();
    websocketpp::lib::error_code ec;
    client::connection_ptr con = ws_client.get_connection("ws://localhost:9002", ec);

    if (ec) {
        std::cerr << "Could not create connection because: " << ec.message() << std::endl;
        return 1; // Exit if connection cannot be established
    }

    ws_client.connect(con);

    // Run the client in a separate thread
    std::thread client_thread([&ws_client]() {
        ws_client.run();
    });

    // Wait for the connection to be established
    {
        std::unique_lock<std::mutex> lock(connection_mutex);
        connection_cv.wait(lock, [] { return connection_open; });
    }

    // Random number generator for selecting columns
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 6);

    // Main loop to send moves
    Player player = Player::CLIENT; // Example player ID
    while (!game_over) { // Continue until the game is over
        if (!my_turn) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait for your turn
            continue;
        }

        int column = dis(gen); // Randomly select a column
        std::cout << "Bot selects column: " << column << std::endl;

        send_move(&ws_client, con->get_handle(), player, column);

        std::cout << "Waiting for server to make a move..." << std::endl;

        // Ensure my_turn is set to false immediately after sending a move
        my_turn = false;
    }

    client_thread.join();
    return 0;
} 