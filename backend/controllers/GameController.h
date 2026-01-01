#pragma once
#include <drogon/HttpController.h>
#include "../services/GameEngine.h"
#include "../db/Repository.h"
#include "../services/Bot.h"

using namespace drogon;

class GameController : public drogon::HttpController<GameController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(GameController::createMatch, "/game/create", Post, Options);
    ADD_METHOD_TO(GameController::getState, "/game/{1}/state", Get, Options);
    ADD_METHOD_TO(GameController::action, "/game/{1}/action", Post, Options);
    ADD_METHOD_TO(GameController::getLeaderboard, "/leaderboard", Get, Options);
    METHOD_LIST_END

    GameController();
    void createMatch(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void getState(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id);
    void action(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id);
    void getLeaderboard(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    void runBotsUntilHuman(int matchId, std::string difficulty);
    void botDecision(BotDecision botDecision, GameState& s, int playerIndex);
    void init(const drogon::orm::DbClientPtr& client){
        repo_ = std::make_unique<Repository>(client);
    }

    GameEngine engine_;
    std::unique_ptr<Repository> repo_;
    std::map<int, GameState> games_;
    // in-memory cache for running matches
};
