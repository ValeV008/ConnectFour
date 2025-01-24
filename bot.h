#ifndef BOT_H
#define BOT_H

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <string>
#include <mutex>
#include <condition_variable>
#include <json/json.h>

/**
 * @enum Player
 * @brief Represents the players in the game.
 */
enum Player {
    NONE = 0,   /**< No player */
    CLIENT = 1, /**< The client player */
    SERVER = 2  /**< The server player */
};

typedef websocketpp::client<websocketpp::config::asio_client> client;

/**
 * @class Bot
 * @brief A class representing a game-playing bot that connects to a server via WebSocket.
 */
class Bot {
public:
    /**
     * @brief Constructor for the Bot class. Initializes the connection state and game state variables.
     */
    Bot();

    /**
     * @brief Runs the bot by connecting to the server at the specified URI and starting the WebSocket client.
     * @param uri The URI of the server to connect to.
     */
    void run(const std::string& uri);

protected:
    /**
     * @brief Handles the event when the WebSocket connection is opened. Sends the player's name to the server.
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     */
    virtual void on_open(client* c, websocketpp::connection_hdl hdl);

    /**
     * @brief Handles incoming messages from the server. Parses the message and calls the appropriate handler based on the message type.
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     * @param msg The message received from the server.
     */
    void on_message(client* c, websocketpp::connection_hdl hdl, client::message_ptr msg);

    /**
     * @brief Abstract method to get the bot's move. Must be implemented by derived classes.
     * @return The column number where the bot wants to place its move.
     */
    virtual int get_move() = 0;

    /**
     * @brief Sends a move to the server. Constructs a JSON message with the move information.
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     * @param column The column number where the bot wants to place its move.
     */
    void send_move(client* c, websocketpp::connection_hdl hdl, int column);

    /**
     * @brief Prints the current state of the game board to the console.
     * @param boardJson The JSON object representing the game board.
     */
    void print_board(const Json::Value& boardJson);

    /**
     * @brief Sends a JSON message to the server using the WebSocket connection.
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     * @param message The JSON message to be sent.
     */
    void send_json_message(client* c, websocketpp::connection_hdl hdl, const Json::Value& message);

    std::string player_name; /**< The name of the player */
    bool last_result_valid;  /**< Indicates if the last move result was valid */

private:
    /**
     * @brief Handles the start of the game. Outputs a message indicating the game has started.
     */
    void handle_game_start();

    /**
     * @brief Handles the result of a move. Updates the game state and checks for win conditions.
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     * @param root The JSON object containing the move result.
     */
    void handle_move_result(client* c, websocketpp::connection_hdl hdl, const Json::Value& root);

    /**
     * @brief Handles the bot's turn to make a move. Prompts for a move and sends it to the server.
     * @param c Pointer to the WebSocket client.
     * @param hdl The connection handle.
     */
    void handle_your_turn(client* c, websocketpp::connection_hdl hdl);

    client ws_client; /**< WebSocket client for communication */
    std::mutex connection_mutex; /**< Mutex for synchronizing connection state */
    std::condition_variable connection_cv; /**< Condition variable for connection state */
    bool connection_open; /**< Indicates if the connection is open */
    bool my_turn; /**< Indicates if it's the bot's turn */
    bool game_over; /**< Indicates if the game is over */
};

#endif // BOT_H