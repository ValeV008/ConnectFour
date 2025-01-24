#ifndef CONNECTFOURGAME_H
#define CONNECTFOURGAME_H

#include <vector>
#include <json/json.h>

const int ROWS = 6;
const int COLUMNS = 7;
const int WIN_CONDITION = 4;

/**
 * @enum Player
 * @brief Represents the players in the game.
 */
enum Player {
    NONE = 0,   /**< No player */
    CLIENT = 1, /**< The client player */
    SERVER = 2  /**< The server player */
};

/**
 * @class ConnectFourGame
 * @brief A class representing the Connect Four game logic.
 */
class ConnectFourGame {
public:
    /**
     * @brief Constructor for the ConnectFourGame class. Initializes the game board.
     */
    ConnectFourGame();

    /**
     * @brief Makes a move for the specified player in the given column.
     * @param player The player making the move.
     * @param column The column where the disc should be dropped.
     * @return True if the move is valid and successful, false otherwise.
     */
    bool make_move(Player player, int column);

    /**
     * @brief Checks if the specified player has won the game.
     * @param player The player to check for a win condition.
     * @return True if the player has won, false otherwise.
     */
    bool check_winner(Player player) const;

    /**
     * @brief Prints the current state of the game board.
     */
    void print_board() const;

    /**
     * @brief Gets the current state of the game board as a JSON object.
     * @return A JSON representation of the game board.
     */
    Json::Value get_board_json() const;

private:
    std::vector<std::vector<int>> board; /**< The game board represented as a 2D vector. */
    int last_move_row = -1; /**< The row index of the last move made. */
    int last_move_col = -1; /**< The column index of the last move made. */

    /**
     * @brief Checks a specific direction for a win condition starting from a given position.
     * @param player The player to check for a win condition.
     * @param row The starting row index.
     * @param column The starting column index.
     * @param d_row The row direction to check.
     * @param d_col The column direction to check.
     * @return True if a win condition is met in the specified direction, false otherwise.
     */
    bool check_direction(Player player, int row, int column, int d_row, int d_col) const;
};

#endif // CONNECTFOURGAME_H 