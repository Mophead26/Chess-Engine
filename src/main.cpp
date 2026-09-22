#include "types.h"
#include "board.h"
#include "movegen.h"
#include "engine.h"
#include "magics.h"
#include "uci.h"
#include <iostream>


int main() {
    // PreCompute lookup tables
    initDistanceToEdge();
    initMagicBitboards();
    initKnightMoves();
    initKingMoves();

    // Initialize the board
    BoardState engineBoard;

    uciLoop(engineBoard);
    
    return 0;
}
