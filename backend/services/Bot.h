#pragma once
#include "GameEngine.h"
#include <memory>

enum class BotDifficulty { EASY, MEDIUM, HARD };

struct BotDecision {
    enum Type { PASS, ROLL, BUILD_ROAD, BUILD_SETTLEMENT, UPGRADE_CITY, PLAY_KNIGHT, MOVE_ROBBER, BUY_DEV_CARD, PLAY_MONOPOLY, PLAY_PLENTY, PLAY_ROAD_BUILDING, END_TURN } type;
    int target1 = -1;// edgeId, vertexId, hexId depending on action
    int target2 = -1;
};

class Bot {
public:
    virtual ~Bot() = default;
    virtual BotDecision decide(GameState const& s, int playerIndex, GameEngine& engine) = 0;
};

std::unique_ptr<Bot> createBot(BotDifficulty d);
