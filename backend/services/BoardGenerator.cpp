#include "BoardGenerator.h"

GameState BoardGenerator::generateInitialState() {
    GameEngine engine;
    return engine.createNewGame();
}
