#pragma once
#include "types.h"
#include "magics.h"
#include <vector>

inline int popLSB(U64& bitboard);

inline U64 getRookAttacks(int square, U64 occupancy);
inline U64 getBishopAttacks(int square, U64 occupancy);
inline U64 getQueenAttacks(int square, U64 occupancy);

void generateWhitePawnMoves(const BoardState& state, std::vector<Move>& moveList);
void generateBlackPawnMoves(const BoardState& state, std::vector<Move>& moveList);

void generateKnightMoves(const BoardState& state, std::vector<Move>& moveList);
void generateKingMoves(const BoardState& state, std::vector<Move>& moveList);
void generateSlidingMoves(const BoardState& state, std::vector<Move>& moveList);

void generateAllMoves(const BoardState& state, std::vector<Move>& moveList);