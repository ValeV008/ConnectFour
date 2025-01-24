#include "RandomJanezBot.h"
#include <iostream>
#include <random>

/**
 * @brief Constructor for the RandomJanezBot class.
 */
RandomJanezBot::RandomJanezBot() {
    player_name = "random Janez";
    last_column = -1;
}

/**
 * @brief Determines the next move for the bot.
 * @return The column number where the bot wants to place its move.
 */
int RandomJanezBot::get_move() {
    // Prioritize the center column if the last move was valid
    if (last_result_valid) {
        last_column = 3;
        return 3;
    }

    // If the last result was invalid or the center column is not an option, choose a random column
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 6);

    int column;
    do {
        column = dis(gen);
    } while (column == last_column); // Ensure a different column is chosen if retrying

    last_column = column;
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

    RandomJanezBot bot;
    bot.run(server_uri);

    return 0;
}