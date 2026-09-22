#include "types.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"


// Material Difference from sideToMove's perspective
int materialScore(BoardState& state) {
    int whiteMaterial = 0;
    int blackMaterial = 0;

    whiteMaterial += numOf(state.bitboards[WhitePawn]) * pawnVal;
    whiteMaterial += numOf(state.bitboards[WhiteKnight]) * knightVal;
    whiteMaterial += numOf(state.bitboards[WhiteBishop]) * bishopVal;
    whiteMaterial += numOf(state.bitboards[WhiteRook]) * rookVal;
    whiteMaterial += numOf(state.bitboards[WhiteQueen]) * queenVal;

    blackMaterial += numOf(state.bitboards[BlackPawn]) * pawnVal;
    blackMaterial += numOf(state.bitboards[BlackKnight]) * knightVal;
    blackMaterial += numOf(state.bitboards[BlackBishop]) * bishopVal;
    blackMaterial += numOf(state.bitboards[BlackRook]) * rookVal;
    blackMaterial += numOf(state.bitboards[BlackQueen]) * queenVal;

    int difference = whiteMaterial - blackMaterial;
    return (state.sideToMove == White) ? difference : -difference;
}






int eval(BoardState& state) {
    return materialScore(state);
}
