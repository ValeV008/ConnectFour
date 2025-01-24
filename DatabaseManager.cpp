#include "DatabaseManager.h"
#include <iostream>

/**
 * @brief Constructor for the DatabaseManager class. Initializes the database connection.
 */
DatabaseManager::DatabaseManager() : db(nullptr) {
    init_database();
}

/**
 * @brief Destructor for the DatabaseManager class. Closes the database connection.
 */
DatabaseManager::~DatabaseManager() {
    if (db) {
        sqlite3_close(db);
    }
}

/**
 * @brief Initializes the database, creating necessary tables if they do not exist.
 */
void DatabaseManager::init_database() {
    int rc = sqlite3_open("connect_four.db", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS players ("
                      "name TEXT PRIMARY KEY, "
                      "elo INTEGER DEFAULT 100);";

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

/**
 * @brief Updates or inserts a player's ELO rating in the database.
 * @param name The name of the player.
 * @param elo_change The change in the player's ELO rating.
 */
void DatabaseManager::update_or_insert_player_elo(const std::string& name, int elo_change) {
    const char* sql = "INSERT INTO players (name, elo) VALUES (?, 100 + ?) "
                      "ON CONFLICT(name) DO UPDATE SET elo = elo + ?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, elo_change);
    sqlite3_bind_int(stmt, 3, elo_change);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to update ELO: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
}

/**
 * @brief Retrieves a player's ELO rating from the database.
 * @param name The name of the player.
 * @return The player's ELO rating.
 */
int DatabaseManager::get_player_elo(const std::string& name) {
    const char* sql = "SELECT elo FROM players WHERE name = ?;";
    sqlite3_stmt* stmt;
    int elo = -1;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            elo = sqlite3_column_int(stmt, 0);
        }
    } else {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return elo;
} 