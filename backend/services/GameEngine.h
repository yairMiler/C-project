#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <array>
#include <string>
#include <optional>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class Resource { Brick, Lumber, Ore, Grain, Wool, None };
enum class Terrain { Hills, Forest, Mountains, Fields, Pasture, Desert };
enum class PortType { None, ThreeToOne, TwoToOneBrick, TwoToOneLumber, TwoToOneGrain, TwoToOneWool, TwoToOneOre };

// Costs
static const std::map<Resource, int> COST_ROAD = { {Resource::Brick,1},{Resource::Lumber,1} };
static const std::map<Resource, int> COST_SETTLEMENT = { {Resource::Brick,1},{Resource::Lumber,1},{Resource::Grain,1},{Resource::Wool,1} };
static const std::map<Resource, int> COST_CITY = { {Resource::Grain,2},{Resource::Ore,3} };
static const std::map<Resource, int> COST_DEVCARD = { {Resource::Grain,1},{Resource::Wool,1},{Resource::Ore,1} };


struct Hex {
    int id;
    Terrain terrain;
    int token; // 2..12, 0 for desert
    bool hasRobber = false;
    std::array<int, 6> adjacentVertices; // indices of the 6 vertices
};

struct Vertex {
    int id;
    double x = 0.0;   
    double y = 0.0;   
    std::optional<int> owner; // player index
    bool isCity = false;
    std::vector<int> adjacentHexes;
    std::vector<int> adjacentEdges;
    PortType port = PortType::None;
};

struct Edge {
    int id;
    std::optional<int> owner;
    std::pair<int, int> vertices; // vertex indices (v1, v2)
    PortType port = PortType::None;
};

struct PlayerState {
    int index;
    std::string userId; // empty for bot
    std::map<Resource, int> resources;
    int setupPlaced = 0;
    int victoryPoints = 0;
    int knightsPlayed = 0;
    int roadsBuilt = 0;
    int settlementsBuilt = 0;
    int citiesBuilt = 0;
    std::vector<std::string> devCards;
    std::vector<int> verticesBuilt;
    bool hasLongestRoad = false;
    bool hasLargestArmy = false;
};

struct GameState {
    std::string id;
    std::vector<Hex> hexes; // 19
    std::vector<Vertex> vertices; // 54
    std::vector<Edge> edges; // 72
    std::array<PlayerState, 4> players;
    int currentPlayer = 0;
    int phase = 2; // 0=setup1, 1=setup2, 2=main
    int setupIndex = 0;
    std::vector<int> setupOrder = {0, 1, 2, 3};
    std::vector<std::string> log;
    int robberHexId = -1;
    int winner = -1;
    int dice = 0;
    int turnDeadline = time(nullptr) + 120;
    std::map<Resource, int> bankResources;
    std::vector<std::string> deck;
    std::pair<int, int> LongestRoadPlayer = {-1,-1};
    std::pair<int, int> BiggestKnightsNumberPlayer = { -1,-1 };
};

class GameEngine {
public:
    GameEngine();
    GameState createNewGame();
    json getStateJson(const GameState& s) const;

    // Actions & validations
    bool rollDice(GameState& s, int playerIndex, int diceSum, std::string& error);
    bool moveRobber(GameState& s, int playerIndex, int targetHexId, std::optional<int> stealFromPlayer, std::string& error);

    bool buildRoad(GameState& s, int playerIndex, int edgeId, std::string& error);
    bool buildSettlement(GameState& s, int playerIndex, int vertexId, std::string& error);
    bool upgradeToCity(GameState& s, int playerIndex, int vertexId, std::string& error);

    bool buyDevCard(GameState& s, int playerIndex, std::string& error);
    bool playKnightCard(GameState& s, int playerIndex, int targetHexId, std::optional<int> stealFromPlayer, std::string& error);

    bool tradeWithBank(GameState& s, int playerIndex, Resource give, int giveAmount, Resource receive, std::string& error, int rate = 4);
    bool endTurn(GameState& s, int playerIndex, std::string& error);

    bool checkVictory(const GameState& s, int playerIndex) const;

    // helper access
    static Resource terrainToResource(Terrain t);
    static Resource stringToResourc(std::string s);

    void initBoard(GameState& s);
    void fillAdjacency(GameState& s);
    void updateLongestRoad(GameState&, int playerIndex, int pathLength);
    void updateBiggestKnights(GameState&, int playerIndex, int knightsNumber);
    void distributeResources(GameState& s, int diceSum);
    bool playerHasResources(const PlayerState& p, const std::map<Resource, int>& cost) const;
    void payResources(GameState& s, PlayerState& p, const std::map<Resource, int>& cost);
    bool isVertexLegalForSettlement(const GameState& s, int vertexId, std::string& error) const;
    bool isEdgeConnectsToPlayer(const GameState& s, int playerIndex, int edgeId) const;
    bool playMonopolyCard(GameState& s, int playerIndex, Resource target, std::string& error);
    bool playPlentyCard(GameState& s, int playerIndex, Resource r1, Resource r2, std::string& error);
    bool playRoadBuildingCard(GameState& s, int playerIndex, int edge1, int edge2, std::string& error);
    int computeLongestRoad(const GameState& s, int playerIndex) const;
    int simulateDiceRoll();
};
