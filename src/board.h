#pragma once

#include <string>
#include "types.h"

// Index 0: N, 1: S, 2: E, 3: W, 4: NW, 5: SE, 6: NE, 7: SW
inline int numSquaresToEdge[64][8]; 

// --- Board Initialization & Parsing ---
void updateOccupancies(BoardState& state);
void initStartingPosition(BoardState& state);
void parseFen(const std::string& fenString, BoardState& state);

// Move Making
bool makeMove(BoardState& state, Move move);
bool isSquareAttacked(int square, int attackingColor, const BoardState& state);

// PreCompute functions
void initDistanceToEdge();
void initKnightMoves();
void initKingMoves();

// --- Debugging ---
void printBoard(const BoardState& state);
void printBitboard(U64 bitboard); 