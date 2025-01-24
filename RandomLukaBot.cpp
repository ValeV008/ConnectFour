#include "RandomLukaBot.h"
#include <iostream>
#include <random>

/**
 * @brief Constructor for the RandomLukaBot class.
 */
RandomLukaBot::RandomLukaBot() {
    player_name = "random Luka";
}

/**
 * @brief Determines the next move for the bot.
 * @return The column number where the bot wants to place its move.
 */
int RandomLukaBot::get_move() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 6);
    int column = dis(gen);
    return column;
}

/**
 * @brief Runs the bot by connecting to the server and playing the game.
 * @param server_uri The URI of the WebSocket server to connect to.
 * @return Exit status of the program.
 */
int main(int argc, char* argv[]) {
    // Check if server URI is provided
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <server_uri> (e.g., ws://localhost:9002)" << std::endl;
        std::cout << "Press Enter to exit...";
        std::cin.get();  // Wait for user input before exiting
        return 1;
    }

    std::string server_uri = argv[1];

    RandomLukaBot bot;
    bot.run(server_uri);

    return 0;
}