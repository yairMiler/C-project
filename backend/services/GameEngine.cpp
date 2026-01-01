// GameEngine.cpp -- full game rules engine including adjacency, validations, resource distribution

#include "GameEngine.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <numeric>


GameEngine::GameEngine() {
    // nothing
}

Resource GameEngine::terrainToResource(Terrain t) {
    switch (t) {
    case Terrain::Hills: return Resource::Brick;
    case Terrain::Forest: return Resource::Lumber;
    case Terrain::Mountains: return Resource::Ore;
    case Terrain::Fields: return Resource::Grain;
    case Terrain::Pasture: return Resource::Wool;
    default: return Resource::None;
    }
}

Resource GameEngine::stringToResourc(std::string s) {
    if (s == "brick")  return Resource::Brick;
    if (s == "lumber") return Resource::Lumber;
    if (s == "ore")    return Resource::Ore;
    if (s == "grain")  return Resource::Grain;
    if (s == "wool")   return Resource::Wool;
    return Resource::None;
}

GameState GameEngine::createNewGame() {
    GameState s;
    initBoard(s);
    // init players
    for (int i = 0; i < 4; i++) {
        s.players[i].index = i;
        s.players[i].userId = "";
        s.players[i].resources.clear();
        s.players[i].victoryPoints = 0;
    }
    // place robber on desert
    for (auto& h : s.hexes) if (h.terrain == Terrain::Desert) { h.hasRobber = true; s.robberHexId = h.id; break; }
    s.currentPlayer = 0;
    s.phase = 0; // setup first piece sequence
    s.log.clear();
    s.id = "local-" + std::to_string(std::rand());
    s.turnDeadline = time(nullptr) + 120;
    return s;
}

json GameEngine::getStateJson(const GameState& s) const {
    json j;

    j["id"] = s.id;
    j["currentPlayer"] = s.currentPlayer;
    j["phase"] = s.phase;
    j["robber"] = s.robberHexId;
    j["winner"] = s.winner;
    j["turnDeadline"] = s.turnDeadline; 
    j["dice"] = s.dice;          

    // ---- Hexes ----
    j["hexes"] = json::array();
    for (auto const& h : s.hexes) {
        json hj;
        hj["id"] = h.id;
        hj["terrain"] = static_cast<int>(h.terrain);
        hj["token"] = h.token;
        hj["robber"] = h.hasRobber;
        hj["adjacentVertices"] = h.adjacentVertices;     
        j["hexes"].push_back(hj);
    }

    // ---- Vertices ----
    j["vertices"] = json::array();
    for (auto const& v : s.vertices) {
        json vj;
        vj["id"] = v.id;
        vj["x"] = v.x;                  
        vj["y"] = v.y;                  
        vj["owner"] = v.owner.has_value() ? v.owner.value() : -1;
        vj["isCity"] = v.isCity;

        vj["adjacentHexes"] = v.adjacentHexes;           
        vj["adjacentEdges"] = v.adjacentEdges;           
        vj["port"] = static_cast<int>(v.port);           

        j["vertices"].push_back(vj);
    }

    // ---- Edges ----
    j["edges"] = json::array();
    for (auto const& e : s.edges) {
        json ej;
        ej["id"] = e.id;
        ej["owner"] = e.owner.has_value() ? e.owner.value() : -1;
        ej["v1"] = e.vertices.first;
        ej["v2"] = e.vertices.second;
        ej["port"] = static_cast<int>(e.port);

        j["edges"].push_back(ej);
    }

    // ---- Players ----
    j["players"] = json::array();
    for (auto const& p : s.players) {
        json pj;
        pj["index"] = p.index;
        pj["userId"] = p.userId;

        pj["victoryPoints"] = p.victoryPoints;
        pj["knightsPlayed"] = p.knightsPlayed;          
        pj["roadsBuilt"] = p.roadsBuilt;                 
        pj["settlementsBuilt"] = p.settlementsBuilt;     
        pj["citiesBuilt"] = p.citiesBuilt;               

        pj["hasLongestRoad"] = p.hasLongestRoad;         
        pj["hasLargestArmy"] = p.hasLargestArmy;         

        pj["verticesBuilt"] = p.verticesBuilt;           
        pj["devCards"] = p.devCards;                     

        pj["resources"] = json::object();
        for (auto const& kv : p.resources)
            pj["resources"][std::to_string((int)kv.first)] = kv.second;

        j["players"].push_back(pj);
    }

    // ---- Bank Resources ----
    j["bankResources"] = json::object();                 
    for (auto const& kv : s.bankResources)
        j["bankResources"][std::to_string((int)kv.first)] = kv.second;

    // ---- Development Deck ----
    j["deck"] = s.deck;                                  

    // ---- Longest Road + Largest Army ----
    j["LongestRoadPlayer"] = {                           
        s.LongestRoadPlayer.first,
        s.LongestRoadPlayer.second
    };

    j["BiggestKnightsNumberPlayer"] = {                  
        s.BiggestKnightsNumberPlayer.first,
        s.BiggestKnightsNumberPlayer.second
    };

    return j;
}


/* ========== BOARD init & adjacency ==========
   We'll create a canonical mapping that matches standard Catan indexing:
   - 19 hexes numbered 0..18
   - 54 vertices 0..53
   - 72 edges 0..71
   We'll initialize hex tile order by axial coordinates (like generator),
   and fill each hex's adjacentVertices and each vertex's adjacentHexes/edges,
   and fill edges with vertex pairs.

   For brevity and correctness we generate adjacency programmatically by positioning hex axial coords and computing shared vertices.
*/

struct Axial { int q, r; };
static std::vector<Axial> makeHexCoordinates() {
    // axial coords for 19-hex standard layout (radius 2)
    std::vector<Axial> coords;
    for (int r = -2; r <= 2; r++) {
        int qmin = std::max(-2, -r - 2);
        int qmax = std::min(2, -r + 2);
        for (int q = qmin; q <= qmax; q++) {
            coords.push_back({ q,r });
        }
    }
    return coords; // size 19
}

// Helper to compute unique vertex positions from hex corners
static std::pair<double, double> hexCorner(double cx, double cy, double size, int corner) {
    double angle_deg = 60 * corner - 30;
    double angle_rad = M_PI / 180.0 * angle_deg;
    return { cx + size * cos(angle_rad), cy + size * sin(angle_rad) };
}

void GameEngine::initBoard(GameState& s) {
    // Create tile resources and numbers randomization
    s.bankResources.clear();
    s.bankResources[Resource::Brick] = 19;
    s.bankResources[Resource::Lumber] = 19;
    s.bankResources[Resource::Grain] = 19;
    s.bankResources[Resource::Wool] = 19;
    s.bankResources[Resource::Ore] = 19;

    s.deck.clear();
    for (int i = 0; i < 14; i++) s.deck.push_back("KNIGHT");
    for (int i = 0; i < 5; i++) s.deck.push_back("VP");
    for (int i = 0; i < 2; i++) s.deck.push_back("ROAD");
    for (int i = 0; i < 2; i++) s.deck.push_back("PLENTY");
    for (int i = 0; i < 2; i++) s.deck.push_back("MONOPOLY");
    std::random_device rd; std::mt19937 g(rd());
    std::shuffle(s.deck.begin(), s.deck.end(), g);

    std::vector<Terrain> terrains;
    terrains.insert(terrains.end(), 4, Terrain::Forest);
    terrains.insert(terrains.end(), 3, Terrain::Hills);
    terrains.insert(terrains.end(), 3, Terrain::Mountains);
    terrains.insert(terrains.end(), 4, Terrain::Fields);
    terrains.insert(terrains.end(), 4, Terrain::Pasture);
    terrains.push_back(Terrain::Desert);

    std::vector<int> numbers = { 5,2,6,3,8,10,9,12,11,4,8,10,9,4,5,6,3,11 };

    // port pairs (24 vertices)
    std::vector<PortType> ports = {
        PortType::TwoToOneGrain, PortType::TwoToOneGrain,
        PortType::ThreeToOne,    PortType::ThreeToOne,
        PortType::TwoToOneLumber, PortType::TwoToOneLumber,
        PortType::TwoToOneBrick,  PortType::TwoToOneBrick,
        PortType::ThreeToOne,    PortType::ThreeToOne,
        PortType::ThreeToOne,    PortType::ThreeToOne,
        PortType::TwoToOneWool,  PortType::TwoToOneWool,
        PortType::ThreeToOne,    PortType::ThreeToOne,
        PortType::TwoToOneOre,   PortType::TwoToOneOre
    };

    // skip amounts between port pairs, according to your board
    std::vector<int> gaps = { 2, 2, 3, 2, 2, 3, 3, 2 };

    std::shuffle(terrains.begin(), terrains.end(), g);
    std::shuffle(numbers.begin(), numbers.end(), g);

    auto coords = makeHexCoordinates(); 
    s.hexes.clear();
    int tIndex = 0, nIndex = 0;
    for (int i = 0; i < (int)coords.size(); ++i) {
        Hex h;
        h.id = i;
        h.terrain = terrains[tIndex++];
        if (h.terrain == Terrain::Desert) h.token = 0;
        else h.token = numbers[nIndex++];
        h.hasRobber = (h.terrain == Terrain::Desert); // Robber starts on Desert
        h.adjacentVertices = { -1,-1,-1,-1,-1,-1 };
        s.hexes.push_back(h);
    }

    // --- FIX: Correct Coordinate Projection (Pointy Top Hexes) ---
    double size = 50.0;
    std::vector<std::pair<double, double>> uniquePts;
    std::vector<std::array<int, 6>> hexVertexIndices(s.hexes.size());

    auto findOrAdd = [&](std::pair<double, double> p) -> int {
        for (int i = 0; i < (int)uniquePts.size(); ++i) {
            double dx = uniquePts[i].first - p.first;
            double dy = uniquePts[i].second - p.second;
            // Use a slightly larger epsilon for floating point safety
            if (std::sqrt(dx*dx + dy*dy) < 0.1) return i;
        }
        uniquePts.push_back(p);
        return (int)uniquePts.size() - 1;
    };

    for (size_t i = 0; i < coords.size(); ++i) {
        // Correct axial to pixel conversion for Pointy Top Hexes
        double cx = size * (sqrt(3.0) * coords[i].q + sqrt(3.0)/2.0 * coords[i].r);
        double cy = size * (1.5 * coords[i].r);

        for (int corner = 0; corner < 6; corner++) {
            // corner 0 is 30 degrees (right-ish), rotating clockwise
            double angle_deg = 60.0 * corner - 30.0;
            double angle_rad = M_PI / 180.0 * angle_deg;
            std::pair<double, double> p = { 
                cx + size * cos(angle_rad), 
                cy + size * sin(angle_rad) 
            };

            int vid = findOrAdd(p);
            hexVertexIndices[i][corner] = vid;
        }
    }

    // --- Vertex and Edge Construction ---
    int V = (int)uniquePts.size(); // Should now be 54
    s.vertices.clear();
    s.vertices.resize(V);
    for (int v = 0; v < V; ++v) {
        s.vertices[v].id = v;
        s.vertices[v].x = uniquePts[v].first;
        s.vertices[v].y = uniquePts[v].second;
        s.vertices[v].port = PortType::None;
    }

    // Assign Adjacency
    std::map<std::pair<int, int>, int> edgeMap;
    std::vector<std::pair<int, int>> edgesVec;

    for (size_t hi = 0; hi < s.hexes.size(); ++hi) {
        for (int corner = 0; corner < 6; corner++) {
            int v1 = hexVertexIndices[hi][corner];
            int v2 = hexVertexIndices[hi][(corner + 1) % 6];

            s.hexes[hi].adjacentVertices[corner] = v1;
            s.vertices[v1].adjacentHexes.push_back((int)hi);

            // Create Edges
            int low = std::min(v1, v2);
            int high = std::max(v1, v2);
            auto key = std::make_pair(low, high);
            if (edgeMap.find(key) == edgeMap.end()) {
                int eId = (int)edgesVec.size();
                edgeMap[key] = eId;
                edgesVec.push_back(key);
            }
        }
    }

    s.edges.clear();
    s.edges.resize(edgesVec.size()); // Should now be 72
    for (int i = 0; i < (int)edgesVec.size(); ++i) {
        s.edges[i].id = i;
        s.edges[i].vertices = edgesVec[i];
        s.vertices[edgesVec[i].first].adjacentEdges.push_back(i);
        s.vertices[edgesVec[i].second].adjacentEdges.push_back(i);
    }


    // --- Port Logic (Ring Detection) ---
std::vector<int> ring;
for (int v = 0; v < (int)s.vertices.size(); v++) { 
    // Ports only go on the outer boundary (vertices touching 1 or 2 hexes) 
    if (s.vertices[v].adjacentHexes.size() < 3) ring.push_back(v);
}

// Calculate geometric center of the ring for sorting
double avgX = 0, avgY = 0;
if (!ring.empty()) {
    for (int v : ring) { avgX += s.vertices[v].x; avgY += s.vertices[v].y; }
    avgX /= ring.size(); avgY /= ring.size();
}

// Sort CCW
std::sort(ring.begin(), ring.end(), [&](int a, int b) { 
    return atan2(s.vertices[a].y - avgY, s.vertices[a].x - avgX) < 
           atan2(s.vertices[b].y - avgY, s.vertices[b].x - avgX);
});

// Helper to check if two vertices are connected by an edge
auto shareEdge = [&](int v1, int v2) { 
    for (int e1 : s.vertices[v1].adjacentEdges) { 
        for (int e2 : s.vertices[v2].adjacentEdges) { 
            if (e1 == e2) return true; 
        } 
    } 
    return false;
};

// Rotate the ring until ring[0] and ring[1] share a physical edge
// This prevents a port from being split across the sorting "seam"
if (ring.size() >= 2) {
    for (int shift = 0; shift < (int)ring.size(); ++shift) { 
        if (shareEdge(ring[0], ring[1])) break; 
        std::rotate(ring.begin(), ring.begin() + 1, ring.end());
    }
}

// FIXED GAPS: 18 port vertices + 12 gap vertices = 30 total ring vertices
// This sequence distributes the ports evenly around the board
std::vector<int> adjustedGaps = { 1, 1, 2, 1, 1, 2, 1, 1, 2 };

// Assign PortTypes
int ringIdx = 0;
int portDataIdx = 0;

// Reset all vertex ports to None first to avoid leftovers
for (auto& v : s.vertices) v.port = PortType::None;

for (int pair = 0; pair < 9; pair++) { 
    if (ringIdx + 1 >= (int)ring.size() || portDataIdx + 1 >= (int)ports.size()) break;

    // Take 2 adjacent vertices from the ring 
    int v_a = ring[ringIdx % ring.size()]; 
    int v_b = ring[(ringIdx + 1) % ring.size()]; 

    s.vertices[v_a].port = ports[portDataIdx++]; 
    s.vertices[v_b].port = ports[portDataIdx++]; 

    // Move the index: 2 for the port vertices + the gap
    ringIdx += 2; 
    if (pair < (int)adjustedGaps.size()) { 
        ringIdx += adjustedGaps[pair]; 
    }
}

// Final edge update: An edge is a port only if BOTH its vertices share the same PortType
for (auto& edge : s.edges) { 
    const auto& v1 = s.vertices.at(edge.vertices.first); 
    const auto& v2 = s.vertices.at(edge.vertices.second); 
    
    if (v1.port != PortType::None && v1.port == v2.port) { 
        edge.port = v1.port; 
    } else { 
        edge.port = PortType::None; 
    }
}

}

void GameEngine::distributeResources(GameState& s, int diceSum) {
    for (auto& h : s.hexes) {
        if (h.token == diceSum && !h.hasRobber) {
            Resource r = terrainToResource(h.terrain);
            if (r == Resource::None) continue;
            for (int vid : h.adjacentVertices) {
                if (vid < 0 || vid >= (int)s.vertices.size()) continue;
                auto& v = s.vertices[vid];
                if (!v.owner.has_value()) continue;
                int owner = v.owner.value();
                int amount = v.isCity ? 2 : 1;
                s.bankResources[r] -= amount;
                s.players[owner].resources[r] += amount;
            }
        }
    }
}

static std::map<Resource, int> makeCopy(const std::map<Resource, int>& m) { return m; }

bool GameEngine::playerHasResources(const PlayerState& p, const std::map<Resource, int>& cost) const {
    for (auto& kv : cost) {
        Resource r = kv.first;
        int need = kv.second;
        auto it = p.resources.find(r);
        int have = (it == p.resources.end()) ? 0 : it->second;
        if (have < need) return false;
    }
    return true;
}
void GameEngine::payResources(GameState& s, PlayerState& p, const std::map<Resource, int>& cost) {
    for (auto& kv : cost) {
        Resource r = kv.first;
        p.resources[r] -= kv.second;
        s.bankResources[kv.first] += kv.second;
        if (p.resources[r] < 0) p.resources[r] = 0;
    }
}

bool GameEngine::isVertexLegalForSettlement(const GameState& s, int vertexId, std::string& error) const {
    if (vertexId < 0 || vertexId >= (int)s.vertices.size()) { error = "invalid vertex"; return false; }
    const auto& v = s.vertices[vertexId];
    if (v.owner.has_value()) { error = "vertex already occupied"; return false; }
    // distance rule: no adjacent vertices are occupied
    for (int edgeId : v.adjacentEdges) {
        auto e = s.edges[edgeId];
        int other = (e.vertices.first == vertexId) ? e.vertices.second : e.vertices.first;
        if (s.vertices[other].owner.has_value()) { error = "must be at least two edges away from other settlements"; return false; }
    }
    return true;
}

bool GameEngine::isEdgeConnectsToPlayer(const GameState& s, int playerIndex, int edgeId) const {
    if (edgeId < 0 || edgeId >= (int)s.edges.size()) return false;
    auto& e = s.edges[edgeId];
    // If either adjacent vertex belongs to player OR an adjacent edge owned by player connects, it's legal.
    if (s.vertices[e.vertices.first].owner.has_value() && s.vertices[e.vertices.first].owner.value() == playerIndex) return true;
    if (s.vertices[e.vertices.second].owner.has_value() && s.vertices[e.vertices.second].owner.value() == playerIndex) return true;
    // check adjacent edges
    for (int v : {e.vertices.first, e.vertices.second}) {
        for (int adjE : s.vertices[v].adjacentEdges) {
            if (adjE == edgeId) continue;
            if (s.edges[adjE].owner.has_value() && s.edges[adjE].owner.value() == playerIndex) return true;
        }
    }
    return false;
}

void GameEngine::updateLongestRoad(GameState& s, int playerIndex, int pathLength) {
    auto& newPlayer = s.players[playerIndex];
    if (s.LongestRoadPlayer.first >= 0) {
        auto& oldPlayer = s.players[s.LongestRoadPlayer.first];
        if (playerIndex != s.LongestRoadPlayer.first && pathLength > s.LongestRoadPlayer.second) {
            s.LongestRoadPlayer = {playerIndex, pathLength};
            oldPlayer.hasLongestRoad = false;
            oldPlayer.victoryPoints -= 2;
            newPlayer.hasLongestRoad = true;
            newPlayer.victoryPoints += 2;
        }
    }
    else {
        newPlayer.hasLongestRoad = true;
        newPlayer.victoryPoints += 2;
    }

}

// compute a simple longest road: DFS counting consecutive owned edges avoiding branches
int GameEngine::computeLongestRoad(const GameState& s, int playerIndex) const {
    int best = 0;
    int E = (int)s.edges.size();
    std::vector<bool> visited(E, false);
    for (int e = 0; e < E; ++e) {
        if (!s.edges[e].owner.has_value() || s.edges[e].owner.value() != playerIndex) continue;
        // try both directions from each edge endpoints
        auto dfs = [&](auto&& self, int currEdge, int fromVertex, std::vector<bool>& used)->int {
            used[currEdge] = true;
            int maxDepth = 0;
            // go to the other vertex of current edge
            int v = (s.edges[currEdge].vertices.first == fromVertex) ? s.edges[currEdge].vertices.second : s.edges[currEdge].vertices.first;
            for (int nextEdge : s.vertices[v].adjacentEdges) {
                if (nextEdge == currEdge) continue;
                if (used[nextEdge]) continue;
                if (!s.edges[nextEdge].owner.has_value() || s.edges[nextEdge].owner.value() != playerIndex) continue;
                int d = self(self, nextEdge, v, used);
                if (d > maxDepth) maxDepth = d;
            }
            used[currEdge] = false;
            return 1 + maxDepth;
            };
        std::vector<bool> usedLocal(E, false);
        int v1 = s.edges[e].vertices.first;
        int v2 = s.edges[e].vertices.second;
        int d1 = dfs(dfs, e, v1, usedLocal);
        int d2 = dfs(dfs, e, v2, usedLocal);
        best = std::max(best, std::max(d1, d2));
    }
    return best;
}

bool GameEngine::buildRoad(GameState& s, int playerIndex, int edgeId, std::string& error) {
    if (edgeId < 0 || edgeId >= (int)s.edges.size()) { error = "invalid edge"; return false; }
    auto& e = s.edges[edgeId];
    if (e.owner.has_value()) { error = "edge already occupied"; return false; }
    if (s.phase == 0 || s.phase == 1) {
        int expectedPlayer = s.setupOrder[s.setupIndex];
        if (playerIndex != expectedPlayer) { error = "not your setup turn"; return false; }
        if (s.players[playerIndex].setupPlaced != 1) { error = "place settlement first"; return false; }}
    auto& player = s.players[playerIndex];
    if (s.phase == 2 && !playerHasResources(player, COST_ROAD)) { error = "not enough resources"; return false; }
    if (player.roadsBuilt >= 15) { error = "You can't build more than 15 roads"; return false; }
    // allow building during setup without connection check: if in main phase, must connect
    if (!isEdgeConnectsToPlayer(s, playerIndex, edgeId)) { error = "must build connected to your road/settlement"; return false; }
    if (s.phase == 2) {payResources(s, player, COST_ROAD);}
    e.owner = playerIndex;
    player.roadsBuilt++;

    // Setup bookkeeping: if in setup and had settlement placed, mark done and advance setupIndex
    if (s.phase==0 || s.phase==1) {
        s.players[playerIndex].setupPlaced = 2;
        s.setupIndex++;
        if (s.setupIndex >= 4) {
            if (s.phase == 0) {
                // move to phase 1, reverse the setupOrder
                std::reverse(s.setupOrder.begin(), s.setupOrder.end());
                s.phase = 1;
                s.setupIndex = 0;
                s.currentPlayer++;
            } else {
                s.phase = 2;
                //s.currentPlayer = s.setupOrder[3]; 
            }
        }
    }
    // update longest road / flags
    int LR = computeLongestRoad(s, playerIndex);
    if (LR >= 5) updateLongestRoad(s, playerIndex, LR);
    return true;
}

bool GameEngine::playRoadBuildingCard(GameState& s, int playerIndex, int edge1, int edge2, std::string& error)
{
    return buildRoad(s, playerIndex, edge1, error) && buildRoad(s, playerIndex, edge2, error);
}

bool GameEngine::buildSettlement(GameState& s, int playerIndex, int vertexId, std::string& error) {
    if (vertexId < 0 || vertexId >= (int)s.vertices.size()) { error = "invalid vertex"; return false; }
    if (s.phase == 0 || s.phase == 1) {
        int expectedPlayer = s.setupOrder[s.setupIndex];
        if (playerIndex != expectedPlayer) { error = "not your setup turn"; return false; }}
    if (!isVertexLegalForSettlement(s, vertexId, error)) return false;
    auto& player = s.players[playerIndex];
    if (s.phase == 2 && !playerHasResources(player, COST_SETTLEMENT)) { error = "not enough resources"; return false; }
    if (player.settlementsBuilt >= 5) { error = "You cannot build more than 5 settlements"; return false; }
    if (s.phase == 2) payResources(s, player, COST_SETTLEMENT);
    s.vertices[vertexId].owner = playerIndex;
    s.vertices[vertexId].isCity = false;
    player.settlementsBuilt++;
    player.verticesBuilt.push_back(vertexId);
    player.victoryPoints += 1;
    if (s.phase==0 || s.phase==1) {s.players[playerIndex].setupPlaced = 1;}
    if (s.phase ==1) { 
        for (auto& h : s.hexes) {
            Resource r = terrainToResource(h.terrain);
            if (r == Resource::None) continue;
            for (int vid : h.adjacentVertices) {
                if (vid < 0 || vid >= (int)s.vertices.size() || vid != vertexId) continue;
                auto& v = s.vertices[vid];
                if (!v.owner.has_value() || !v.owner != playerIndex) continue;
                int owner = v.owner.value();
                int amount = v.isCity ? 2 : 1;
                s.bankResources[r] -= amount;
                s.players[owner].resources[r] += amount;
           }
       }
    } 
    return true;
}

bool GameEngine::upgradeToCity(GameState& s, int playerIndex, int vertexId, std::string& error) {
    if (vertexId < 0 || vertexId >= (int)s.vertices.size()) { error = "invalid vertex"; return false; }
    auto& v = s.vertices[vertexId];
    if (!v.owner.has_value() || v.owner.value() != playerIndex) { error = "not your settlement"; return false; }
    if (v.isCity) { error = "already a city"; return false; }
    auto& player = s.players[playerIndex];
    if (!playerHasResources(player, COST_CITY)) { error = "not enough resources"; return false; }
    if (player.citiesBuilt >= 4) { error = "You cannot build more than 4 cities"; return false; }
    payResources(s, player, COST_CITY);
    v.isCity = true;
    player.citiesBuilt++;
    player.settlementsBuilt--;
    player.victoryPoints += 1;
    return true;
}

bool GameEngine::buyDevCard(GameState& s, int playerIndex, std::string& error) {
    auto& p = s.players[playerIndex];
    if (!playerHasResources(p, COST_DEVCARD)) { error = "not enough resources"; return false; }
    payResources(s, p, COST_DEVCARD);

    if (s.deck.empty()) { error = "no dev cards left"; return false; }
    std::string card = s.deck.back(); s.deck.pop_back();
    p.devCards.push_back(card);
    if (card == "VP") p.victoryPoints += 1;
    return true;
}

void GameEngine::updateBiggestKnights(GameState& s, int playerIndex, int knightsNumber) {
    auto& newPlayer = s.players[playerIndex];
    if (s.LongestRoadPlayer.first >= 0) {
        auto& oldPlayer = s.players[s.LongestRoadPlayer.first];
        if (playerIndex != s.LongestRoadPlayer.first && knightsNumber > s.LongestRoadPlayer.second) {
            s.LongestRoadPlayer = { playerIndex, knightsNumber };
            oldPlayer.victoryPoints -= 2;
            newPlayer.victoryPoints += 2;
            oldPlayer.hasLargestArmy = false;
            newPlayer.hasLargestArmy = true;
        }
    }
    else {
        newPlayer.victoryPoints += 2;
        newPlayer.hasLargestArmy = true;
    }

}

bool GameEngine::playKnightCard(GameState& s, int playerIndex, int targetHexId, std::optional<int> stealFromPlayer, std::string& error) {
    auto& p = s.players[playerIndex];
    auto it = std::find(p.devCards.begin(), p.devCards.end(), "KNIGHT");
    if (it == p.devCards.end()) { error = "no knight"; return false; }
    p.devCards.erase(it);
    p.knightsPlayed++;
    updateBiggestKnights(s, playerIndex, p.knightsPlayed);
    // move robber
    return moveRobber(s, playerIndex, targetHexId, stealFromPlayer, error);
}

bool GameEngine::moveRobber(GameState& s, int playerIndex, int targetHexId, std::optional<int> stealFromPlayer, std::string& error) {
    if (targetHexId < 0 || targetHexId >= (int)s.hexes.size()) { error = "invalid hex"; return false; }
    if (s.hexes[targetHexId].hasRobber) { error = "robber already there"; return false; }
    s.hexes[s.robberHexId].hasRobber = false;
    s.robberHexId = targetHexId;
    s.hexes[targetHexId].hasRobber = true;
    // steal random resource from specified victim if they have settlement adjacent to that hex
    if (stealFromPlayer.has_value()) {
        int victim = stealFromPlayer.value();
        if (victim < 0 || victim >= 4) { error = "invalid steal victim"; return false; }
        auto& pv = s.players[victim];
        std::vector<Resource> available;
        for (auto& kv : pv.resources) if (kv.second > 0) available.push_back(kv.first);
        if (!available.empty()) {
            std::random_device rd; std::mt19937 g(rd());
            std::uniform_int_distribution<> d(0, (int)available.size() - 1);
            Resource r = available[d(g)];
            pv.resources[r]--;
            s.players[playerIndex].resources[r]++;
        }
    }
    return true;
}

bool GameEngine::tradeWithBank(GameState& s, int playerIndex, Resource give, int giveAmount, Resource receive, std::string& error, int rate) {
    auto& p = s.players[playerIndex];

    // ---- Calculate best available rate ----
    rate = 4; // default

    for (int vid : p.verticesBuilt) {
        auto port = s.vertices[vid].port;

        if (port == PortType::ThreeToOne)
            rate = std::min(rate, 3);

        if (port == PortType::TwoToOneBrick && give == Resource::Brick)
            rate = std::min(rate, 2);
        if (port == PortType::TwoToOneLumber && give == Resource::Lumber)
            rate = std::min(rate, 2);
        if (port == PortType::TwoToOneGrain && give == Resource::Grain)
            rate = std::min(rate, 2);
        if (port == PortType::TwoToOneWool && give == Resource::Wool)
            rate = std::min(rate, 2);
        if (port == PortType::TwoToOneOre && give == Resource::Ore)
            rate = std::min(rate, 2);
    }

    // ---- Validate ----
    if (giveAmount < rate) {
        error = "Not enough to trade";
        return false;
    }
    if (p.resources[give] < giveAmount) {
        error = "You do not have the required resources";
        return false;
    }
    if (s.bankResources[receive] <= 0) {
        error = "Bank does not have the requested resource";
        return false;
    }

    // ---- Execute trade ----
    p.resources[give] -= giveAmount;
    p.resources[receive] += 1;
    s.bankResources[give] += giveAmount;
    s.bankResources[receive] -= 1;

    return true;
}

bool GameEngine::playMonopolyCard(GameState& s, int playerIndex, Resource target, std::string& error)
{
    int total = 0;

    for (int i = 0; i < (int)s.players.size(); i++) {
        if (i == playerIndex) continue;

        int amount = s.players[i].resources[target];
        s.players[i].resources[target] = 0;
        total += amount;
    }

    s.players[playerIndex].resources[target] += total;
    return true;
}

bool GameEngine::playPlentyCard(GameState& s, int playerIndex,Resource r1, Resource r2,std::string& error)
{
    if (s.bankResources[r1] <= 0 || s.bankResources[r2] <= 0) {
        error = "Bank does not have enough resources";
        return false;
    }

    s.bankResources[r1]--;
    s.bankResources[r2]--;
    s.players[playerIndex].resources[r1]++;
    s.players[playerIndex].resources[r2]++;

    return true;
}

bool GameEngine::rollDice(GameState& s, int playerIndex, int diceSum, std::string& error) {
    if (s.currentPlayer != playerIndex) { error = "not your turn"; return false; }
    if (diceSum < 2 || diceSum>12) { error = "invalid dice"; return false; }
    if (diceSum == 7) {
        // discards: players with >7 cards discard half
        for (auto& pl : s.players) {
            int total = 0; for (auto& kv : pl.resources) total += kv.second;
            if (total > 7) {
                int toDiscard = total / 2;
                while (toDiscard > 0) {
                    auto it = std::max_element(pl.resources.begin(), pl.resources.end(),
                        [](auto& a, auto& b) { return a.second < b.second; });
                    if (it == pl.resources.end() || it->second == 0) break;
                    int take = std::min(toDiscard, it->second);
                    it->second -= take;
                    toDiscard -= take;
                }
            }
        }
        return true;
    }
    else {
        distributeResources(s, diceSum);
        return true;
    }
}

int GameEngine::simulateDiceRoll() {
    // Weighted distribution of sums (36 total = 6�6 dice outcomes)
    static const std::vector<std::pair<int, int>> outcomes = {
        { 2, 1 },
        { 3, 2 },
        { 4, 3 },
        { 5, 4 },
        { 6, 5 },
        { 7, 6 },
        { 8, 5 },
        { 9, 4 },
        {10, 3 },
        {11, 2 },
        {12, 1 }
    };

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::discrete_distribution<> dist({
        1, // 2
        2, // 3
        3, // 4
        4, // 5
        5, // 6
        6, // 7
        5, // 8
        4, // 9
        3, // 10
        2, // 11
        1  // 12
        });

    int index = dist(gen);
    return outcomes[index].first;
}

bool GameEngine::endTurn(GameState& s, int playerIndex, std::string& error) {
    std::string str;
    if (s.currentPlayer != playerIndex) { error = "not your turn"; return false; }
    if (checkVictory(s, playerIndex)) {
        // leave, but allow engine to mark winner externally
        s.winner = playerIndex;
    }

    if (s.phase !=1) {s.currentPlayer = (s.currentPlayer + 1) % 4;
    } else {s.currentPlayer = (s.currentPlayer - 1) % 4;}
   
    if (s.phase == 2){
        int dice = simulateDiceRoll();
        s.dice = dice;
        rollDice(s, playerIndex, dice, str);
    }

    if (s.phase == 2) {s.turnDeadline = time(nullptr) + 60;
    } else { s.turnDeadline = time(nullptr) + 120; }
    return true;
}

bool GameEngine::checkVictory(const GameState& s, int playerIndex) const {
    return s.players[playerIndex].victoryPoints >= 10;
}
