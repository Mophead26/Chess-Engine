#pragma once
#include "types.h"
#include "board.h"
#include <vector>
#include <iostream>
#include <chrono>


int alphaBeta(BoardState& state, int depth, int alpha, int beta);
Move getBestMove(BoardState& state, int depth);

U64 perft(BoardState& state, int depth);
void perftDivide(BoardState& state, int depth);