#pragma once
#include "Bot.h"
#include "GameEngine.h"
#include <limits>
#include <optional>
#include <vector>

class BotHard : public Bot {
public:
    int maxDepth = 2;   // user can change

    BotHard(int depth = 2) : maxDepth(depth) {}

BotDecision decide(GameState const& s, int pid, GameEngine& engine) override {
    if (s.winner != -1) return { BotDecision::PASS, -1 };

    double bestScore = -1e18;
    // Default to ending turn if no better move is found
    BotDecision bestMove = { BotDecision::END_TURN, -1 };

    auto actions = generateActions(s, pid, engine);

    for (auto& action : actions) {
        GameState copy = s;
        simulateAction(copy, pid, action, engine);

        // If the first action was END_TURN, we decrease depth immediately
        int nextDepth = (action.type == BotDecision::END_TURN) ? maxDepth - 1 : maxDepth;
        bool nextIsMax = (action.type == BotDecision::END_TURN) ? false : true;

        double v = minimax(copy, nextDepth, nextIsMax, pid, engine);

        if (v > bestScore) {
            bestScore = v;
            bestMove = action;
        }
    }
    return bestMove;
}

private: 

// ------------------------------------------------------ 
// MINIMAX 
// ------------------------------------------------------ 
double minimax(GameState& s, int depth, bool isMax, int originalBotId, GameEngine& engine) 
{ 
    // 1. Base Case: Depth limit or game over
    if (depth <= 0 || s.winner != -1) 
        return evaluate(s, originalBotId); 

    int pid = s.currentPlayer; 
    if (pid < 0 || pid >= (int)s.players.size()) return evaluate(s, originalBotId);

    // 2. Generate actions
    auto actions = generateActions(s, pid, engine); 
    if (actions.empty()) return evaluate(s, originalBotId);

    double best = isMax ? -1e18 : 1e18;

    // 3. Limit Search Breadth (The "1-hour hang" fix)
    int limit = 0;
    int maxBreadth = (s.phase == 2) ? 6 : 4; // Phase 2 allows more options

    for (auto& act : actions) {
        if (++limit > maxBreadth) break;

        GameState sim = s;
        simulateAction(sim, pid, act, engine);

        // Check if the turn passed to another player
        bool turnChanged = (sim.currentPlayer != s.currentPlayer);
        
        // MANDATORY: depth - 1. This prevents all Stack Overflows.
        // If turn changed, we flip isMax to simulate the opponent's turn.
        double val = minimax(sim, depth - 1, turnChanged ? !isMax : isMax, originalBotId, engine);
        
        if (isMax) best = (std::max)(best, val);
        else       best = (std::min)(best, val);
    }

    return (best <= -1e17 || best >= 1e17) ? evaluate(s, originalBotId) : best;
}


    // -------------------------------------------------------
    // EVALUATION FUNCTION
    // -------------------------------------------------------
    double evaluate(GameState const& s, int botId) {
        if (botId >= 4) botId = 3;
        auto const& me = s.players[botId];
        int oppId = (botId + 1) % 4;
        if (s.phase !=1) {oppId = (botId + 1) % 4;
        } else {oppId = (botId - 1 + 4) % 4;}

        double vp = me.victoryPoints;
        double resProd = computeResourceProduction(s, botId);
        double portV = computePortScore(s, botId);
        double roadPot = computeRoadPotential(s, botId);
        double cityPot = computeCityPotential(s, botId);
        double oppThreat = computeOpponentThreat(s, oppId);

        return vp
            + 0.3 * resProd
            + 0.2 * portV
            + 0.2 * roadPot
            + 0.2 * cityPot
            - 0.4 * oppThreat;
    }

    // -------------------------------------------------------
    // ACTION GENERATION
    // -------------------------------------------------------
    std::vector<BotDecision> generateActions(GameState const& s, int pid, GameEngine& engine) {
    if (pid >= 4) pid = 3;
    std::string err;

    std::vector<BotDecision> out;
    
    auto const& p = s.players[pid];

    // --- PHASE 0 & 1: SETUP LOGIC ---
    if (s.phase != 2) {
        if (p.setupPlaced == 0) { // Must build Settlement
            for (auto const& v : s.vertices) {
                if (!v.owner.has_value() && engine.isVertexLegalForSettlement(s, v.id, err)) {
                    out.push_back({ BotDecision::BUILD_SETTLEMENT, v.id, -1 });
                    if (out.size() > 6) break; // Limit search breadth
                }
            }
        } else { // Must build Road attached to that Settlement
            if (!p.verticesBuilt.empty()) {
                int lastV = p.verticesBuilt.back();
                for (int edgeId : s.vertices[lastV].adjacentEdges) {
                    if (edgeId != -1 && !s.edges[edgeId].owner.has_value()) {
                        out.push_back({ BotDecision::BUILD_ROAD, edgeId, -1 });
                    }
                }
            }
        }
        return out; // Setup phase has no other legal moves
    }

    // --- PHASE 2: NORMAL GAMEPLAY ---

    // 1. Build Settlements (Only if affordable and valid)
    if (engine.playerHasResources(p, COST_SETTLEMENT)) {
        for (auto const& v : s.vertices) {
            if (!v.owner.has_value() && engine.isVertexLegalForSettlement(s, v.id, err)) {
                out.push_back({ BotDecision::BUILD_SETTLEMENT, v.id, -1 });
                if (out.size() > 4) break; 
            }
        }
    }

    // 2. Build Cities (Upgrade existing settlements)
    if (engine.playerHasResources(p, COST_CITY)) {
        for (int vid : p.verticesBuilt) {
            if (vid >= 0 && vid < (int)s.vertices.size() && !s.vertices[vid].isCity) {
                out.push_back({ BotDecision::UPGRADE_CITY, vid, -1 });
            }
        }
    }

    // 3. Build Roads
    if (engine.playerHasResources(p, COST_ROAD)) {
        for (auto const& e : s.edges) {
            if (!e.owner.has_value()) {
                // Heuristic: only check roads connected to our existing network
                out.push_back({ BotDecision::BUILD_ROAD, e.id, -1 });
                if (out.size() > 10) break;
            }
        }
    }

    // 4. Buy Dev Card (Corrected COST_DEV_CARD)
    if (engine.playerHasResources(p, COST_DEVCARD)) {
        out.push_back({ BotDecision::BUY_DEV_CARD, -1, -1 });
    }

    // 5. Play Knight (Aggressive Robber movement)
    if (playerHasDev(p, "Knight")) {
        int knightCount = 0;
        for (auto const& h : s.hexes) {
            if (!h.hasRobber && h.token != 0) { // Only move to productive hexes
                for (int vIdx : h.adjacentVertices) {
                    if (vIdx >= 0 && vIdx < (int)s.vertices.size() && s.vertices[vIdx].owner.has_value()) {
                        int victim = s.vertices[vIdx].owner.value();
                        if (victim != pid) {
                            out.push_back({ BotDecision::PLAY_KNIGHT, h.id, victim });
                            if (++knightCount > 2) break; // Don't check every single hex
                        }
                    }
                }
            }
            if (knightCount > 2) break;
        }
    }

    // 6. Play Monopoly
    if (playerHasDev(p, "Monopoly")) {
        for (int r = 0; r < 5; r++) out.push_back({ BotDecision::PLAY_MONOPOLY, r, -1 });
    }

    // 7. Play Year of Plenty
    if (playerHasDev(p, "Plenty")) {
        // Only suggest high-value resource pairs to save time
        out.push_back({ BotDecision::PLAY_PLENTY, 0, 1 }); // e.g., Brick/Wood
        out.push_back({ BotDecision::PLAY_PLENTY, 3, 4 }); // e.g., Wheat/Ore
    }

    // 8. Play Road Building
    if (playerHasDev(p, "RoadBuilding")) {
        int rbCount = 0;
        for (auto const& e : s.edges) {
            if (!e.owner.has_value()) {
                out.push_back({ BotDecision::PLAY_ROAD_BUILDING, e.id, -1 });
                if (++rbCount > 3) break; 
            }
        }
    }

    // 9. Mandatory: End Turn
    out.push_back({ BotDecision::END_TURN, -1 });

    return out;
        
    }

    // -------------------------------------------------------
    // APPLY ACTION TO STATE
    // -------------------------------------------------------
    void simulateAction(GameState& s, int pid, BotDecision const& a, GameEngine& engine) {
        if (pid >= 4) pid = 3;
        std::string err;

        switch (a.type) {
        case BotDecision::BUILD_SETTLEMENT:
            engine.buildSettlement(s, pid, a.target1, err);
            break;
        case BotDecision::BUILD_ROAD:
            engine.buildRoad(s, pid, a.target1, err);
            break;
        case BotDecision::UPGRADE_CITY:
            engine.upgradeToCity(s, pid, a.target1, err);
            break;
        case BotDecision::BUY_DEV_CARD:
            engine.buyDevCard(s, pid, err);
            break;
        case BotDecision::PLAY_KNIGHT:
            engine.playKnightCard(s, pid, a.target1, a.target2, err);
            break;
        case BotDecision::PLAY_MONOPOLY:
            engine.playMonopolyCard(s, pid, (Resource)a.target1, err);
            break;
        case BotDecision::PLAY_PLENTY: {
    
            engine.playPlentyCard(s, pid, (Resource)a.target1, (Resource)a.target2, err);
            break;
        }
        case BotDecision::PLAY_ROAD_BUILDING:
            engine.playRoadBuildingCard(s, pid, a.target1, a.target2, err);
            break;
        case BotDecision::END_TURN:
            engine.endTurn(s, pid, err);
            break;
        default:
            break;
        }
    }

    bool playerHasDev(PlayerState const& p, std::string const& dev) {
        return std::find(p.devCards.begin(), p.devCards.end(), dev) != p.devCards.end();
    }

    // -------------------------------------------------------
    // EVALUATION HELPERS (simple placeholders)
    // -------------------------------------------------------
    double computeResourceProduction(GameState const& s, int pid) {
        double sum = 0;
        for (auto vid : s.players[pid].verticesBuilt) {
            auto const& v = s.vertices[vid];
            for (auto h : v.adjacentHexes) {
                int tok = s.hexes[h].token;
                sum += diceProb(tok);
            }
        }
        return sum;
    }

    double diceProb(int tok) {
        static std::map<int, double> p = {
            {2,1}, {3,2}, {4,3}, {5,4}, {6,5},
            {8,5}, {9,4}, {10,3}, {11,2}, {12,1}
        };
        return p[tok] / 5.0;
    }

    double computePortScore(GameState const& s, int pid) { 
            double score = 0.0;

            for (int vid : s.players[pid].verticesBuilt)
            {
                auto port = s.vertices[vid].port;

                switch (port)
                {
                case PortType::ThreeToOne:
                    score += 1.0;
                    break;

                case PortType::TwoToOneBrick:
                case PortType::TwoToOneLumber:
                case PortType::TwoToOneGrain:
                case PortType::TwoToOneWool:
                case PortType::TwoToOneOre:
                    score += 2.0;
                    break;

                default: break;
                }
            }
            return score;
     } 

     double computeRoadPotential(GameState const& s, int pid) {

            double result = 0.0;

            for (auto const& e : s.edges)
            {
                if (e.owner == pid)
                {
                    // Roads connected ? branching potential
                    int v1 = e.vertices.first;
                    int v2 = e.vertices.second;

                    // count empty adjacent edges
                    int free1 = 0, free2 = 0;

                    for (int eid : s.vertices[v1].adjacentEdges)
                        if (!s.edges[eid].owner.has_value())
                            free1++;

                    for (int eid : s.vertices[v2].adjacentEdges)
                        if (!s.edges[eid].owner.has_value())
                            free2++;

                    result += 0.5 * (free1 + free2);
                }
            }
            return result;
     } 

    double computeCityPotential(GameState const& s, int pid) { 
        double score = 0.0;

        auto tokenWeight = [](int tok) {
            if (tok == 6 || tok == 8) return 5.0;
            if (tok == 5 || tok == 9) return 4.0;
            if (tok == 4 || tok == 10) return 3.0;
            if (tok == 3 || tok == 11) return 2.0;
            if (tok == 2 || tok == 12) return 1.0;
            return 0.0;
            };

        for (int vid : s.players[pid].verticesBuilt)
        {
            if (!s.vertices[vid].isCity)
            {
                // settlement => potential upgrade
                double vscore = 0.0;

                for (int hid : s.vertices[vid].adjacentHexes)
                    vscore += tokenWeight(s.hexes[hid].token);

                score += vscore;
            }
        }

        return score;
    }

    double computeOpponentThreat(GameState const& s, int oid) { return s.players[oid].victoryPoints; }
};

