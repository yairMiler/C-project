#include "Bot.h"
#include "BotHard.h"
#include "GameEngine.h"
#include <random>
#include <algorithm>

class BotEasy : public Bot {
public:
    BotDecision decide(GameState const& s, int pid, GameEngine& engine) override {
        // if can build settlement, build at first legal vertex, else build road, else roll
        for (auto const& v : s.vertices) {
            if (!v.owner.has_value()) {
                std::string err;
                GameState copy = s;
                if (engine.playerHasResources(s.players[pid], COST_SETTLEMENT) && engine.isVertexLegalForSettlement(s, v.id, err)) {
                    return { BotDecision::BUILD_SETTLEMENT, v.id };
                }
            }
        }
        for (auto const& e : s.edges) {
            if (!e.owner.has_value()) {
                if (engine.playerHasResources(s.players[pid], COST_ROAD)) return {BotDecision::BUILD_ROAD, e.id, -1};
            }
        }
        return { BotDecision::ROLL, -1, -1 };
    }
};

class BotMedium : public Bot {
public:
    BotDecision decide(GameState const& s, int pid, GameEngine& engine) override {
        // compute vertex score by token probability
        double bestScore = 0; int bestVid = -1;
        auto tokenWeight = [](int tok) {
            if (tok == 6 || tok == 8) return 5.0;
            if (tok == 5 || tok == 9) return 4.0;
            if (tok == 4 || tok == 10) return 3.0;
            if (tok == 3 || tok == 11) return 2.0;
            if (tok == 2 || tok == 12) return 1.0;
            return 0.0;
            };
        for (auto const& v : s.vertices) {
            if (v.owner.has_value()) continue;
            double score = 0.0;
            for (int h : v.adjacentHexes) {
                int tok = s.hexes[h].token;
                score += tokenWeight(tok);
            }
            if (score > bestScore) { bestScore = score; bestVid = v.id; }
        }
        if (bestVid != -1 && engine.playerHasResources(s.players[pid], COST_SETTLEMENT)) return { BotDecision::BUILD_SETTLEMENT, bestVid, -1 };
        return { BotDecision::ROLL, -1, -1 };
    }
};

std::unique_ptr<Bot> createBot(BotDifficulty d) {
    if (d == BotDifficulty::EASY) return std::make_unique<BotEasy>();
    if (d == BotDifficulty::MEDIUM) return std::make_unique<BotMedium>();
    if (d == BotDifficulty::HARD) return std::make_unique<BotHard>(300);
    return std::make_unique<BotEasy>();
}
