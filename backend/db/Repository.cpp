#include "Repository.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <drogon/orm/Exception.h> 

Repository::Repository(DbClientPtr client){
    dbClient_ = client;

}

void Repository::prepareStatements()
{
  
}

int Repository::createUserIfNotExists(const std::string& googleId, const std::string& displayName)
{
    auto result = dbClient_->execSqlSync(
        "INSERT INTO users (google_id, display_name) VALUES ($1, $2) "
        "ON CONFLICT (google_id) DO UPDATE SET display_name=EXCLUDED.display_name "
        "RETURNING id",
        googleId,
        displayName
    );

    if (result.empty()) return -1;
    
    return result[0]["id"].as<int>();
}

int Repository::createMatch(const std::string& mode, const std::string& difficulty)
{
    auto result = dbClient_->execSqlSync(
        "INSERT INTO matches (mode, difficulty) VALUES ($1, $2) RETURNING id",
        mode,
        difficulty
    );
   
    return result[0]["id"].as<int>();
}

void Repository::saveGameState(int matchId, const json& state)
{
    std::string s = state.dump(); // nlohmann::json dump to string

    dbClient_->execSqlSync(
        "INSERT INTO game_state (match_id, json_state) "
        "VALUES ($1, $2) "
        "ON CONFLICT (match_id) DO UPDATE SET json_state = $2",
        matchId,
        s
    );
}

json Repository::loadGameState(int matchId)
{
    auto result = dbClient_->execSqlSync(
        "SELECT json_state FROM game_state WHERE match_id=$1",
        matchId
    );

    if (result.empty())
        return json();

    // FIX 3: Get the value as a std::string from the first row, "json_state" column
    std::string json_state_str = result[0]["json_state"].as<std::string>();
    return json::parse(json_state_str);
}

std::string Repository::loadGameLevel(int matchId){
        auto result = dbClient_->execSqlSync(
        "SELECT difficulty FROM matches WHERE match_id=$1",
        matchId
    );
    return result[0]["matches"].as<std::string>();
}

std::string Repository::loadGameMode(int matchId){
        auto result = dbClient_->execSqlSync(
        "SELECT mode FROM matches WHERE match_id=$1",
        matchId
    );
    return result[0]["matches"].as<std::string>();
}

void Repository::finalizeMatch(int matchId, int winnerUserId)
{
    dbClient_->execSqlSync(
        "UPDATE matches SET end_time=now(), winner_id=$2 WHERE id=$1",
        matchId,
        winnerUserId
    );
}

void Repository::upsertUser(const std::string& name) {
    
    dbClient_->execSqlSync(
        "INSERT INTO users (display_name, games_played) VALUES ($1, 1) "
        "ON CONFLICT (display_name) DO UPDATE SET games_played = users.games_played + 1",
        name
    );
}

void Repository::recordWin(const std::string& name, int points) {
    
    dbClient_->execSqlSync(
        "UPDATE users SET wins = wins + 1, total_points = total_points + $2 "
        "WHERE display_name = $1",
        name,
        points
    );
}

Json::Value Repository::getLeaderboard() {
    // 1. Execute SQL to get top users
    auto res = dbClient_->execSqlSync(
        "SELECT display_name, games_played, wins, total_points "
        "FROM users ORDER BY wins DESC, total_points DESC LIMIT 20"
    );

    // 2. Convert database rows into a JSON array
    Json::Value list(Json::arrayValue);
    for (const auto& row : res) {
        Json::Value user;
        user["name"] = row["display_name"].as<std::string>();
        user["games"] = row["games_played"].as<int>();
        user["wins"] = row["wins"].as<int>();
        user["points"] = row["total_points"].as<int>();
        list.append(user);
    }
    return list;
}

void Repository::runMigrations() { 
    try { 
        std::string path = "C:\\Games\\Catan\\backend\\db\\Migrations\\V1__init.sql"; 
        std::ifstream file(path); 
        
        if (file.is_open()) { 
            std::stringstream buffer; 
            buffer << file.rdbuf(); 
            std::string sql = buffer.str();

            if (sql.empty()) {
                LOG_ERROR << "Migration file is empty!";
                return;
            }

            // Execute the SQL
            dbClient_->execSqlSync(sql); 
            LOG_INFO << "Migrations executed successfully."; 
        } else { 
            LOG_ERROR << "Migration file not found at: " << path; 
        } 
    } catch (const std::exception &e) { 
        // Changed to LOG_ERROR so you can see why the SQL failed
        LOG_ERROR << "Migration SQL Error: " << e.what(); 
    }
}



