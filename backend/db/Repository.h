#pragma once
#include "Db.h"
#include <string>
#include <nlohmann/json.hpp>
#include <drogon/orm/DbClient.h> // Include Drogon ORM headers

using json = nlohmann::json;
using drogon::orm::DbClientPtr;

class Repository {
public:
    // CHANGE 1: Constructor takes a Drogon DbClientPtr
    Repository(DbClientPtr client);

    int createUserIfNotExists(const std::string& googleId, const std::string& displayName);
    // ... other function declarations remain the same ...
    int createMatch(const std::string& mode, const std::string& difficulty);
    void saveGameState(int matchId, const json& state);
    json loadGameState(int matchId);
    void finalizeMatch(int matchId, int winnerUserId);
    void upsertUser(const std::string& name);
    void recordWin(const std::string& name, int points);
    std::string loadGameLevel(int matchId);
    std::string loadGameMode(int matchId);
    Json::Value getLeaderboard();
    void runMigrations();

private:
    void prepareStatements(); // We might move away from pqxx::prepare statements soon with this approach

    // CHANGE 2: Store the DbClientPtr instead of a pqxx::connection
     drogon::orm::DbClientPtr dbClient_;
};

