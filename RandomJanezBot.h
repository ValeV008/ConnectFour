#ifndef RANDOMJANEZBOT_H
#define RANDOMJANEZBOT_H

#include "Bot.h"

/**
 * @class RandomJanezBot
 * @brief A bot that makes semi random moves in the Connect Four game.
 */
class RandomJanezBot : public Bot {
public:
    /**
     * @brief Constructor for the RandomJanezBot class.
     */
    RandomJanezBot();

protected:
    /**
     * @brief Determines the next move for the bot.
     * @return The column number where the bot wants to place its move.
     */
    int get_move() override;

private:
    int last_column; /**< The last column where the bot placed a move. */
};

#endif // RANDOMJANEZBOT_H