#include "types.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"

// Returns a value between 0 (Endgame) and 24 (Opening/Midgame)
int getGamePhase(const BoardState& state) {
    int phase = 0;
    
    // Combine White and Black bitboards to count total pieces left
    phase += __builtin_popcountll(state.bitboards[WhiteKnight] | state.bitboards[BlackKnight]) * 1;
    phase += __builtin_popcountll(state.bitboards[WhiteBishop] | state.bitboards[BlackBishop]) * 1;
    phase += __builtin_popcountll(state.bitboards[WhiteRook]   | state.bitboards[BlackRook])   * 2;
    phase += __builtin_popcountll(state.bitboards[WhiteQueen]  | state.bitboards[BlackQueen])  * 4;
    
    // Safety cap in case of multiple pawn promotions
    if (phase > 24) phase = 24; 
    
    return phase;
}

int evaluatePiece(U64 bitboard, int pieceValue, const int* pst, bool isWhite) {
    int score = 0;
    while (bitboard) {
        int square = popLSB(bitboard);

        score += pieceValue;

        if (isWhite) {
            score += pst[square];
        } else {
            score += pst[square ^ 56];
        }
    }
    return score;
}

int getScore(BoardState& state) {
    int whiteScore = 0;
    int blackScore = 0;

    // Phase of the game to interpolate between Mid & End
    int phase = getGamePhase(state);

    // white Score
    whiteScore += evaluatePiece(state.bitboards[WhitePawn], pawnVal, pawnPST, true);
    whiteScore += evaluatePiece(state.bitboards[WhiteKnight], knightVal, knightPST, true);
    whiteScore += evaluatePiece(state.bitboards[WhiteBishop], bishopVal, bishopPST, true);
    whiteScore += evaluatePiece(state.bitboards[WhiteRook], rookVal, rookPST, true);
    whiteScore += evaluatePiece(state.bitboards[WhiteQueen], queenVal, queenPST, true);

    int wKingMg = evaluatePiece(state.bitboards[WhiteKing], kingVal, kingMidPST, true);
    int wKingEg = evaluatePiece(state.bitboards[WhiteKing], kingVal, kingEndPST, true);
    whiteScore += (wKingMg * phase + wKingEg * (24 - phase)) / 24;

    // Black Score
    blackScore += evaluatePiece(state.bitboards[BlackPawn], pawnVal, pawnPST, false);
    blackScore += evaluatePiece(state.bitboards[BlackKnight], knightVal, knightPST, false);
    blackScore += evaluatePiece(state.bitboards[BlackBishop], bishopVal, bishopPST, false);
    blackScore += evaluatePiece(state.bitboards[BlackRook], rookVal, rookPST, false);
    blackScore += evaluatePiece(state.bitboards[BlackQueen], queenVal, queenPST, false);

    int bKingMg = evaluatePiece(state.bitboards[BlackKing], kingVal, kingMidPST, false);
    int bKingEg = evaluatePiece(state.bitboards[BlackKing], kingVal, kingEndPST, false);
    blackScore += (bKingMg * phase + bKingEg * (24 - phase)) / 24;

    int evaluation = whiteScore - blackScore;
    return (state.sideToMove == White) ? evaluation : -evaluation;
}

int eval(BoardState& state) {
    return getScore(state);
}
