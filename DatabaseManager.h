#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <string>
#include <sqlite3.h>
#include <vector>
#include <map>

/**
 * @class DatabaseManager
 * @brief A class responsible for managing database operations.
 */
class DatabaseManager {
public:
    /**
     * @brief Constructor for the DatabaseManager class. Initializes the database connection.
     */
    DatabaseManager();

    /**
     * @brief Destructor for the DatabaseManager class. Closes the database connection.
     */
    ~DatabaseManager();

    /**
     * @brief Initializes the database, creating necessary tables if they do not exist.
     */
    void init_database();

    /**
     * @brief Updates or inserts a player's ELO rating in the database.
     * @param name The name of the player.
     * @param elo_change The change in the player's ELO rating.
     */
    void update_or_insert_player_elo(const std::string& name, int elo_change);

    /**
     * @brief Retrieves a player's ELO rating from the database.
     * @param name The name of the player.
     * @return The player's ELO rating.
     */
    int get_player_elo(const std::string& name);

private:
    sqlite3* db; /**< Pointer to the SQLite database. */
    std::string database_name; /**< The name of the database. */
};

#endif // DATABASEMANAGER_H 