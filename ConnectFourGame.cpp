#include "ConnectFourGame.h"
#include <iostream>

/**
 * @brief Constructor for the ConnectFourGame class. Initializes the game board.
 */
ConnectFourGame::ConnectFourGame() {
    board.resize(ROWS, std::vector<int>(COLUMNS, Player::NONE));
}

/**
 * @brief Makes a move for the specified player in the given column.
 * @param player The player making the move.
 * @param column The column where the disc should be dropped.
 * @return True if the move is valid and successful, false otherwise.
 */
bool ConnectFourGame::make_move(Player player, int column) {
    std::cout << "Making move for player " << player << " in column " << column << std::endl;

    if (column < 0 || column >= COLUMNS)
        return false;
    
    for (int row = ROWS - 1; row >= 0; --row) {
        if (board[row][column] == Player::NONE) {
            board[row][column] = player;
            last_move_row = row;
            last_move_col = column;
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if the specified player has won the game.
 * @param player The player to check for a win condition.
 * @return True if the player has won, false otherwise.
 */
bool ConnectFourGame::check_winner(Player player) const {
    return check_direction(player, last_move_row, last_move_col, 1, 0) || // Horizontal
           check_direction(player, last_move_row, last_move_col, 0, 1) || // Vertical
           check_direction(player, last_move_row, last_move_col, 1, 1) || // Forward Diagonal
           check_direction(player, last_move_row, last_move_col, 1, -1);  // Backward Diagonal
}

/**
 * @brief Prints the current state of the game board.
 */
void ConnectFourGame::print_board() const {
    for (const auto& row : board) {
        for (int cell : row) {
            std::cout << (cell == Player::NONE ? "." : (cell == Player::CLIENT ? "X" : "O")) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

/**
 * @brief Gets the current state of the game board as a JSON object.
 * @return A JSON representation of the game board.
 */
Json::Value ConnectFourGame::get_board_json() const {
    Json::Value boardJson(Json::arrayValue);
    for (const auto& row : board) {
        Json::Value rowJson(Json::arrayValue);
        for (int cell : row) {
            rowJson.append(cell);
        }
        boardJson.append(rowJson);
    }
    return boardJson;
}

/**
 * @brief Checks a specific direction for a win condition starting from a given position.
 * @param player The player to check for a win condition.
 * @param row The starting row index.
 * @param column The starting column index.
 * @param d_row The row direction to check.
 * @param d_col The column direction to check.
 * @return True if a win condition is met in the specified direction, false otherwise.
 */
bool ConnectFourGame::check_direction(Player player, int row, int column, int d_row, int d_col) const {
    int count = 0;
    for (int i = -WIN_CONDITION + 1; i < WIN_CONDITION; ++i) {
        int r = row + i * d_row;
        int c = column + i * d_col;
        if (r >= 0 && r < ROWS && c >= 0 && c < COLUMNS && board[r][c] == player) {
            if (++count == WIN_CONDITION) return true;
        } else {
            count = 0;
        }
    }
    return false;
} 