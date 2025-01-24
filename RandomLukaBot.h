#ifndef RANDOMLUKABOT_H
#define RANDOMLUKABOT_H

#include "Bot.h"

/**
 * @class RandomLukaBot
 * @brief A bot that makes semi random moves in the Connect Four game.
 */
class RandomLukaBot : public Bot {
public:
    /**
     * @brief Constructor for the RandomLukaBot class.
     */
    RandomLukaBot();

protected:
    /**
     * @brief Determines the next move for the bot.
     * @return The column number where the bot wants to place its move.
     */
    int get_move() override;
};

#endif // RANDOMLUKABOT_H