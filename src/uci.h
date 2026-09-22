#pragma once
#include "types.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"
#include "engine.h"

Move parseMove(const std::string& moveStr, const BoardState& state);
void parsePosition(const std::string& commandLine, BoardState& currentState);
void parseGo(const std::string& commandLine, BoardState& currentState);
void uciLoop(BoardState& currentState);