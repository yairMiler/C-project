#include "GameController.h"
#include <drogon/drogon.h>
#include "../services/BoardGenerator.h"
#include "../services/Bot.h"
#include <sstream>
#include <optional>
// We are using Drogon's built-in Json::Value (jsoncpp)
// We assume 'json' alias is defined as nlohmann::json in Repository.h/GameEngine.h

GameController::GameController() {
    //auto client = drogon::app().getFastDbClient("default");
    //repo_ = std::make_unique<Repository>(client);
    //std::cout << "THREAD ID = " << std::this_thread::get_id() << std::endl;


    // This is the correct, robust way to access the configured DB client in Drogon
    //drogon::orm::DbClientPtr clientPtr = drogon::app().getDbClient();

    // Pass the DbClientPtr to the repository (Requires Repository.h/cpp update)
    //repo_ = std::make_unique<Repository>(clientPtr);
    // Engine default constructed

    // In your GameController constructor or initialization:

// In your GameController constructor or initialization:
// This runs once per second for the entire lifetime of the app
   /* drogon::app().getLoop()->runEvery(1.0, [this]() {
       time_t now = time(nullptr);
    
       // Iterate through all active games
       for (auto& [id, s] : games_) {
        
           if (now >= s.turnDeadline) {
              std::string err;
              int pid = s.currentPlayer;
              auto bot = createBot(BotDifficulty::HARD);

            if (s.players[pid].setupPlaced == 0){
                BotDecision d = bot->decide(s, pid, engine_);
                botDecision(d, s, pid);
                d = bot->decide(s, pid, engine_);
                botDecision(d, s, pid);
              }else if(s.players[pid].setupPlaced == 1){
                BotDecision d = bot->decide(s, pid, engine_);
                botDecision(d, s, pid);}

            engine_.endTurn(s, pid, err);  

             repo_->saveGameState(id, engine_.getStateJson(s));
        }
    }
});*/
    
}

void GameController::createMatch(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    //auto client = Db::get();
    //repo_ = std::make_unique<Repository>(client);
            if (!repo_) {
        auto client = drogon::app().getDbClient("default");
        repo_ = std::make_unique<Repository>(client);
    }


    if (!repo_) {

        auto resp = HttpResponse::newHttpJsonResponse(Json::Value());
        resp->setStatusCode(k500InternalServerError);
        resp->setBody("Database not connected");
        callback(resp);
        return; 
    }

    repo_->runMigrations();

    auto body = req->getJsonObject(); // body is std::shared_ptr<Json::Value>

    std::string mode = "vs_bots";
    std::string botLevel = "easy";
    std::string name = "yyyy";

    // Use jsoncpp syntax: isMember() and asString()
    if (body && body->isMember("mode")) {
        mode = (*body)["mode"].asString();
    }

    if (body && body->isMember("botLevel")) {
        botLevel = (*body)["botLevel"].asString();
    }

    if (body && body->isMember("playerName")) {
        name = (*body)["playerName"].asString();
    }

    int matchId = repo_->createMatch(mode, botLevel);
    //int matchId = 1;
    GameState s = BoardGenerator::generateInitialState();
    s.id = std::to_string(matchId);
    games_[matchId] = s;

     if(mode == "vs_bots") { 
        s.players[1].userId = name;
        s.players[1].userId = "bot1";
        s.players[2].userId = "bot2";
        s.players[3].userId = "bot3";
        repo_->upsertUser(name);
     }


    // We assume engine_.getStateJson(s) returns nlohmann::json, 
    // and that the repository handles saving that nlohmann::json object correctly.
    json stateJson_nlohmann = engine_.getStateJson(s);
    repo_->saveGameState(matchId, stateJson_nlohmann);

    // Convert nlohmann::json back to Json::Value for the Drogon response
    Json::Value jsoncpp_response;
    Json::Reader reader;
    reader.parse(stateJson_nlohmann.dump(), jsoncpp_response);

    auto resp = HttpResponse::newHttpJsonResponse(jsoncpp_response);
    callback(resp);
}

void GameController::getState(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id) {
    if (games_.count(id)) {
        auto& s = games_[id];
        json stateJson_nlohmann = engine_.getStateJson(s);
        Json::Value jsoncpp_response;
        Json::Reader reader;
        reader.parse(stateJson_nlohmann.dump(), jsoncpp_response);
        callback(HttpResponse::newHttpJsonResponse(jsoncpp_response));
        return;
    }

    // j is nlohmann::json type returned from the repo function
    json j = repo_->loadGameState(id);

    if (j.is_null()) {
        // Create Json::Value directly for the error response
        Json::Value errorResp;
        errorResp["error"] = "not found";
        callback(HttpResponse::newHttpJsonResponse(errorResp));
        return;
    }

    // Convert nlohmann::json back to Json::Value for the Drogon response
    Json::Value jsoncpp_response;
    Json::Reader reader;
    reader.parse(j.dump(), jsoncpp_response);
    callback(HttpResponse::newHttpJsonResponse(jsoncpp_response));
}

void GameController::action(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int id) {
    auto body = req->getJsonObject();
    if (!body) {
        Json::Value e;
        e["error"] = "invalid json";
        callback(HttpResponse::newHttpJsonResponse(e));
        return;
    }

    // Convert jsoncpp -> nlohmann
    json body_n = json::parse(body->toStyledString());

    // Ensure game exists
    if (!games_.count(id)) {
        json j = repo_->loadGameState(id);
        if (j.is_null()) {
            Json::Value e;
            e["error"] = "not found";
            callback(HttpResponse::newHttpJsonResponse(e));
            return;
        }
        GameState s = engine_.createNewGame();
        s.id = std::to_string(id);
        games_[id] = s;
    }

    GameState& s = games_[id];

    std::string action = body_n["action"].get<std::string>();
    std::string err;
    bool ok = false;
    int pid = s.currentPlayer;

    // =============================
    //         ACTION HANDLERS
    // =============================

    if (action == "auto_play") {
    time_t now = time(nullptr);
    
    // Safety check: Only run if the deadline has actually passed
    if (now >= s.turnDeadline) {
        int pid = s.currentPlayer;
        auto bot = createBot(BotDifficulty::HARD);
        std::string err;

        // 1. Initial Setup logic (matching your provided snippet)
        if (s.players[pid].setupPlaced == 0) {
            BotDecision d1 = bot->decide(s, pid, engine_);
            botDecision(d1, s, pid);
            BotDecision d2 = bot->decide(s, pid, engine_);
            botDecision(d2, s, pid);
        } else if (s.players[pid].setupPlaced == 1) {
            BotDecision d = bot->decide(s, pid, engine_);
            botDecision(d, s, pid);
        }

        // 2. Force the end of turn
        engine_.endTurn(s, pid, err);
    }
    }

    else if (action == "roll") {
        int dice = engine_.simulateDiceRoll();
        ok = engine_.rollDice(s, pid, dice, err);
    }

    else if (action == "build_settlement") {
        int v = body_n["vertex"].get<int>();
        ok = engine_.buildSettlement(s, pid, v, err);
    }

    else if (action == "upgrade_city") {
        int v = body_n["vertex"].get<int>();
        ok = engine_.upgradeToCity(s, pid, v, err);
    }

    else if (action == "build_road") {
        int e = body_n["edge"].get<int>();
        ok = engine_.buildRoad(s, pid, e, err);
    }

    else if (action == "buy_dev") {
        ok = engine_.buyDevCard(s, pid, err);
    }

    else if (action == "move_robber") {
        int hex = body_n["hex"].get<int>();
        std::optional<int> steal;

        if (body_n.contains("steal"))
            steal = body_n["steal"].get<int>();

        ok = engine_.moveRobber(s, pid, hex, steal, err);
    }

    // ---- DEV CARDS ----

    else if (action == "play_knight") {
        int hex = body_n["hex"].get<int>();
        std::optional<int> steal;
        if (body_n.contains("steal"))
            steal = body_n["steal"].get<int>();

        ok = engine_.playKnightCard(s, pid, hex, steal, err);
    }

    else if (action == "play_monopoly") {
        std::string r = body_n["resource"].get<std::string>();
        Resource rr = engine_.stringToResourc(r);
        ok = engine_.playMonopolyCard(s, pid, rr, err);
    }

    else if (action == "play_plenty") {
        Resource r1 = engine_.stringToResourc(body_n["r1"].get<std::string>());
        Resource r2 = engine_.stringToResourc(body_n["r2"].get<std::string>());
        ok = engine_.playPlentyCard(s, pid, r1, r2, err);
    }

    else if (action == "play_road_building") {
        int e1 = body_n["edge1"].get<int>();
        int e2 = body_n["edge2"].get<int>();
        ok = engine_.playRoadBuildingCard(s, pid, e1, e2, err);
    }

    // ---- BANK TRADE ----

    else if (action == "trade_with_bank") {
        Resource give = engine_.stringToResourc(body_n["give"].get<std::string>());
        int giveAmount = body_n["amount"].get<int>();
        Resource receive = engine_.stringToResourc(body_n["receive"].get<std::string>());

        int rate = 4;
        if (body_n.contains("rate"))
            rate = body_n["rate"].get<int>();

        ok = engine_.tradeWithBank(s, pid, give, giveAmount, receive, err, rate);
    }

    else if (action == "end_turn") {
        ok = engine_.endTurn(s, pid, err);
    }

    else {
        err = "unknown action";
    }


    // =============================
    //        SAVE + RESPOND
    // =============================

    json response_n = engine_.getStateJson(s);
    repo_->saveGameState(id, response_n);

    response_n["ok"] = ok;
    if (!ok) response_n["error"] = err;

    // convert nlohmann -> jsoncpp
    Json::Value final_cpp;
    Json::Reader reader;
    reader.parse(response_n.dump(), final_cpp);

    callback(HttpResponse::newHttpJsonResponse(final_cpp));

    std::string difficulty = repo_->loadGameLevel(id);
    std::string mode = repo_->loadGameMode(id);

    if(mode == "vs_bots" && s.players[pid].setupPlaced == 2) { 

        int loopGuard = 0;
        while (s.winner == -1 || loopGuard++ < 3) {
            runBotsUntilHuman(id, difficulty);

            json response_n = engine_.getStateJson(s);
            repo_->saveGameState(id, response_n);

            response_n["ok"] = ok;
            if (!ok) response_n["error"] = err;

           // convert nlohmann -> jsoncpp
           Json::Value final_cpp;
           Json::Reader reader;
           reader.parse(response_n.dump(), final_cpp);

           callback(HttpResponse::newHttpJsonResponse(final_cpp));
        }  
     }
     if (s.winner != -1){
        std::string uid = s.players[pid].userId;
        bool isBot = (uid.rfind("bot:",0) == 0) || uid.empty() == false && uid.substr(0,3)=="bot";
        if(!isBot){repo_->recordWin(uid, s.players[pid].victoryPoints);}
        json response_n = engine_.getStateJson(s);
        repo_->saveGameState(id, response_n);

        response_n["ok"] = ok;
        if (!ok) response_n["error"] = err;

        // convert nlohmann -> jsoncpp
        Json::Value final_cpp;
        Json::Reader reader;
        reader.parse(response_n.dump(), final_cpp);

        callback(HttpResponse::newHttpJsonResponse(final_cpp));  
     }
}

void GameController::getLeaderboard(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback) {
    try {
        Json::Value data = repo_->getLeaderboard();
        callback(HttpResponse::newHttpJsonResponse(data));
    } catch (const std::exception &e) {
        Json::Value error;
        error["error"] = e.what();
        callback(HttpResponse::newHttpJsonResponse(error));
    }
}

void GameController::runBotsUntilHuman(int matchId, std::string difficulty){
    auto &s = games_.at(matchId);

        int pid = s.currentPlayer;
        // if this player is human (userId not bot-like) break
        std::string uid = s.players[pid].userId;
        bool isBot = (uid.rfind("bot:",0) == 0) || uid.empty() == false && uid.substr(0,3)=="bot";
        if (!isBot) return;

        auto bot = createBot(BotDifficulty::EASY);
        if (difficulty == "hard"){
            bot = createBot(BotDifficulty::HARD);
        } else if (difficulty == "medium"){
            bot = createBot(BotDifficulty::MEDIUM);
        }
        
        BotDecision d = bot->decide(s, pid, engine_);

        std::string err;
        bool ok = false;
        switch (d.type) {
        case BotDecision::BUILD_SETTLEMENT:
            engine_.buildSettlement(s, pid, d.target1, err);
            break;
        case BotDecision::BUILD_ROAD:
            engine_.buildRoad(s, pid, d.target1, err);
            break;
        case BotDecision::UPGRADE_CITY:
            engine_.upgradeToCity(s, pid, d.target1, err);
            break;
        case BotDecision::BUY_DEV_CARD:
            engine_.buyDevCard(s, pid, err);
            break;
        case BotDecision::PLAY_KNIGHT:
            engine_.playKnightCard(s, pid, d.target1, d.target2, err);
            break;
        case BotDecision::PLAY_MONOPOLY:
            engine_.playMonopolyCard(s, pid, (Resource)d.target1, err);
            break;
        case BotDecision::PLAY_PLENTY: {
    
            engine_.playPlentyCard(s, pid, (Resource)d.target1, (Resource)d.target2, err);
            break;
        }
        case BotDecision::PLAY_ROAD_BUILDING:
            engine_.playRoadBuildingCard(s, pid, d.target1, d.target2, err);
            break;
        case BotDecision::END_TURN:
            engine_.endTurn(s, pid, err);
            break;
        default:
            break;
        }

}

void GameController::botDecision(BotDecision d, GameState& s, int pid){
        std::string err;
        bool ok = false;
        switch (d.type) {
        case BotDecision::BUILD_SETTLEMENT:
            engine_.buildSettlement(s, pid, d.target1, err);
            break;
        case BotDecision::BUILD_ROAD:
            engine_.buildRoad(s, pid, d.target1, err);
            break;
        case BotDecision::UPGRADE_CITY:
            engine_.upgradeToCity(s, pid, d.target1, err);
            break;
        case BotDecision::BUY_DEV_CARD:
            engine_.buyDevCard(s, pid, err);
            break;
        case BotDecision::PLAY_KNIGHT:
            engine_.playKnightCard(s, pid, d.target1, d.target2, err);
            break;
        case BotDecision::PLAY_MONOPOLY:
            engine_.playMonopolyCard(s, pid, (Resource)d.target1, err);
            break;
        case BotDecision::PLAY_PLENTY: {
    
            engine_.playPlentyCard(s, pid, (Resource)d.target1, (Resource)d.target2, err);
            break;
        }
        case BotDecision::PLAY_ROAD_BUILDING:
            engine_.playRoadBuildingCard(s, pid, d.target1, d.target2, err);
            break;
        case BotDecision::END_TURN:
            engine_.endTurn(s, pid, err);
            break;
        default:
            break;
        }
    }



