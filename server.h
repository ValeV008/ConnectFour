#ifndef CONNECTFOURSERVER_H
#define CONNECTFOURSERVER_H

#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include "ConnectFourGame.h"
#include "DatabaseManager.h"
#include <string>
#include <mutex>
#include <condition_variable>
#include <json/json.h>

typedef websocketpp::server<websocketpp::config::asio> server;

/**
 * @class ConnectFourServer
 * @brief A WebSocket server that manages Connect Four game sessions.
 */
class ConnectFourServer {
public:
    /**
     * @brief Gets the instance of the ConnectFourServer.
     * @return The instance of the ConnectFourServer.
     */
    static ConnectFourServer& getInstance();

    /**
     * @brief Starts the server and begins listening for connections.
     */
    void run();

private:
    /**
     * @brief Constructor for the ConnectFourServer class.
     */
    ConnectFourServer();

    /**
     * @brief Destructor for the ConnectFourServer class.
     */
    ~ConnectFourServer();

    // Delete copy constructor and assignment operator to prevent copying
    ConnectFourServer(const ConnectFourServer&) = delete;
    ConnectFourServer& operator=(const ConnectFourServer&) = delete;

    /**
     * @brief Sends a JSON message to a client.
     * @param hdl The connection handle.
     * @param message The JSON message to send.
     */
    void send_json_message(websocketpp::connection_hdl hdl, const Json::Value& message);

    /**
     * @brief Makes a server move.
     */
    void make_server_move();

    /**
     * @brief Handles new WebSocket connection requests.
     * @param hdl The connection handle.
     */
    void on_open(websocketpp::connection_hdl hdl);

    /**
     * @brief Handles WebSocket connection closures.
     * @param hdl The connection handle.
     */
    void on_close(websocketpp::connection_hdl hdl);

    /**
     * @brief Handles incoming messages from clients.
     * @param hdl The connection handle.
     * @param msg The received message.
     */
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);

    /**
     * @brief Handles a player's name.
     * @param player_name The name of the player.
     */
    void handle_player_name(const std::string& player_name);

    /**
     * @brief Handles a client move.
     * @param column The column to place the piece.
     */
    void handle_client_move(const std::string& column);

    server ws_server; /**< The WebSocket server instance. */
    Player current_player;
    bool client_connected;
    bool game_over;
    std::mutex connection_mutex;
    std::condition_variable connection_cv;
    websocketpp::connection_hdl client_hdl;
    ConnectFourGame game; /**< The current game instance. */
    std::string player_name;
    DatabaseManager db_manager; /**< Database manager for player ratings. */
};

#endif // CONNECTFOURSERVER_H 