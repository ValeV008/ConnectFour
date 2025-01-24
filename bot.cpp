#include "Bot.h"
#include <iostream>

/**
 * @brief Constructor for the Bot class. Initializes the connection state and game state variables.
 */
Bot::Bot(): connection_open(false), my_turn(false), game_over(false), last_result_valid(true) {}

/**
 * @brief Runs the bot by connecting to the server at the specified URI and starting the WebSocket client.
 * @param uri The URI of the server to connect to.
 */
void Bot::run(const std::string& uri) {
    ws_client.clear_access_channels(websocketpp::log::alevel::all);
    ws_client.set_access_channels(websocketpp::log::alevel::app);
    ws_client.clear_error_channels(websocketpp::log::elevel::all);

    ws_client.set_message_handler(bind(&Bot::on_message, this, &ws_client, std::placeholders::_1, std::placeholders::_2));
    ws_client.set_open_handler(bind(&Bot::on_open, this, &ws_client, std::placeholders::_1));

    ws_client.init_asio();
    websocketpp::lib::error_code ec;
    client::connection_ptr con = ws_client.get_connection(uri, ec);

    if (ec) {
        std::cerr << "Could not create connection because: " << ec.message() << std::endl;
        return;
    }

    ws_client.connect(con);

    std::thread client_thread([this]() {
        ws_client.run();
    });

    {
        std::unique_lock<std::mutex> lock(connection_mutex);
        connection_cv.wait(lock, [this] { return connection_open; });
    }

    client_thread.join();
}

/**
 * @brief Handles the event when the WebSocket connection is opened. Sends the player's name to the server.
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 */
void Bot::on_open(client* c, websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(connection_mutex);
    connection_open = true;
    connection_cv.notify_one();

    Json::Value name_message;
    name_message["type"] = "player_name";
    name_message["name"] = player_name;

    send_json_message(c, hdl, name_message);
}

/**
 * @brief Handles incoming messages from the server. Parses the message and calls the appropriate handler based on the message type.
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 * @param msg The message received from the server.
 */
void Bot::on_message(client* c, websocketpp::connection_hdl hdl, client::message_ptr msg) {
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
 * @brief Handles the start of the game. Outputs a message indicating the game has started.
 */
void Bot::handle_game_start() {
    std::cout << "The game has started. You are playing as 'X'." << std::endl;
    std::cout << "Waiting for server to make a move..." << std::endl;
}

/**
 * @brief Handles the result of a move. Updates the game state and checks for win conditions.
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 * @param root The JSON object containing the move result.
 */
void Bot::handle_move_result(client* c, websocketpp::connection_hdl hdl, const Json::Value& root) {
    if (root.isMember("error")) {
        std::cout << "Error: " << root["error"].asString() << std::endl;
        my_turn = true;
        last_result_valid = false;
        handle_your_turn(c, hdl);
        return;
    }

    last_result_valid = true;

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
        game_over = true;
    }
}

/**
 * @brief Handles the bot's turn to make a move. Prompts for a move and sends it to the server.
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 */
void Bot::handle_your_turn(client* c, websocketpp::connection_hdl hdl) {
    my_turn = true;
    std::cout << "It's your turn!" << std::endl;

    int column = get_move();
    send_move(c, hdl, column);

    std::cout << "Bot played in column " << column << ". Waiting for server to make a move..." << std::endl;

    my_turn = false;
}

/**
 * @brief Sends a move to the server. Constructs a JSON message with the move information.
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 * @param column The column number where the bot wants to place its move.
 */
void Bot::send_move(client* c, websocketpp::connection_hdl hdl, int column) {
    Json::Value move;
    move["type"] = "move";
    move["column"] = column;

    send_json_message(c, hdl, move);
}

/**
 * @brief Sends a JSON message to the server using the WebSocket connection.
 * @param c Pointer to the WebSocket client.
 * @param hdl The connection handle.
 * @param message The JSON message to be sent.
 */
void Bot::send_json_message(client* c, websocketpp::connection_hdl hdl, const Json::Value& message) {
    Json::StreamWriterBuilder writer;
    std::string message_str = Json::writeString(writer, message);
    c->send(hdl, message_str, websocketpp::frame::opcode::text);
}

/**
 * @brief Prints the current state of the game board to the console.
 * @param boardJson The JSON object representing the game board.
 */
void Bot::print_board(const Json::Value& boardJson) {
    for (const auto& row : boardJson) {
        for (const auto& cell : row) {
            std::cout << (cell.asInt() == 0 ? "." : (cell.asInt() == 1 ? "X" : "O")) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}