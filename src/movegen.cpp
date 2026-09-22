#include "types.h"
#include "board.h"
#include "magics.h"
#include <vector>

// Finds the index of the lowest '1' bit, then clears it.
inline int popLSB(U64& bitboard) {
    int square = __builtin_ctzll(bitboard); // Find the bit index
    bitboard &= bitboard - 1;               // Clear the lowest set bit
    return square;
}


// Magic Bitboard Lookups
inline U64 getRookAttacks(int square, U64 occupancy) {
    occupancy &= rookMasks[square];
    occupancy *= rookMagics[square];
    occupancy >>= (64 - rookRelevantBits[square]);
    return rookAttackTable[square][occupancy];
}
inline U64 getBishopAttacks(int square, U64 occupancy) {
    occupancy &= bishopMasks[square];
    occupancy *= bishopMagics[square];
    occupancy >>= (64 - bishopRelevantBits[square]);
    return bishopAttackTable[square][occupancy];
}
inline U64 getQueenAttacks(int square, U64 occupancy) {
    return getRookAttacks(square, occupancy) | getBishopAttacks(square, occupancy);
}



void generateWhitePawnMoves(const BoardState& state, std::vector<Move>& moveList) {
    U64 whitePawns = state.bitboards[WhitePawn];
    U64 emptySquares = ~state.occupancies[Both];
    U64 blackPieces = state.occupancies[Black];

    // ------------------ Generate the moves -------------------------------------
    U64 singlePushes = (whitePawns << 8) & emptySquares;
    U64 doublePushes = (singlePushes << 8) & emptySquares & rank4;

    // Captures (excludes the edges so pawns dont teleport to the other side of the board)
    U64 attacksLeft = (whitePawns & ~fileA) << 7 & blackPieces;
    U64 attacksRight = (whitePawns & ~fileH) << 9 & blackPieces;

    // En Passant captures
    if (state.enPassantSquare != NoSquare) {
        U64 epMask = 1ULL << state.enPassantSquare;
        attacksLeft |= (whitePawns & ~fileA) << 7 & epMask;
        attacksRight |= (whitePawns & ~fileH) << 9 & epMask;
    }

    // ------------------ Pack the moves into the struct -------------------------------------
    // Single Pushes
    while (singlePushes) {
        int targetSquare = popLSB(singlePushes);
        int startSquare = targetSquare - 8;

        if ((1ULL << targetSquare) & rank8) {
            moveList.push_back(Move{startSquare, targetSquare, WhiteQueen});
            moveList.push_back(Move{startSquare, targetSquare, WhiteRook});
            moveList.push_back(Move{startSquare, targetSquare, WhiteBishop});
            moveList.push_back(Move{startSquare, targetSquare, WhiteKnight});
        } else {
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }

    // Double Pushes
    while (doublePushes) {
        int targetSquare = popLSB(doublePushes);
        int startSquare = targetSquare - 16;
        moveList.push_back(Move{startSquare, targetSquare, NoPiece});
    }

    // Attacks Left
    while (attacksLeft) {
        int targetSquare = popLSB(attacksLeft);
        int startSquare = targetSquare - 7;

        if ((1ULL << targetSquare) & rank8) {
            moveList.push_back(Move{startSquare, targetSquare, WhiteQueen});
            moveList.push_back(Move{startSquare, targetSquare, WhiteRook});
            moveList.push_back(Move{startSquare, targetSquare, WhiteBishop});
            moveList.push_back(Move{startSquare, targetSquare, WhiteKnight});
        } else {
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }

    // Attacks Right
    while (attacksRight) {
        int targetSquare = popLSB(attacksRight);
        int startSquare = targetSquare - 9;

        if ((1ULL << targetSquare) & rank8) {
            moveList.push_back(Move{startSquare, targetSquare, WhiteQueen});
            moveList.push_back(Move{startSquare, targetSquare, WhiteRook});
            moveList.push_back(Move{startSquare, targetSquare, WhiteBishop});
            moveList.push_back(Move{startSquare, targetSquare, WhiteKnight});
        } else {
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }
}

void generateBlackPawnMoves(const BoardState& state, std::vector<Move>& moveList) {
    U64 blackPawns = state.bitboards[BlackPawn];
    U64 emptySquares = ~state.occupancies[Both];
    U64 whitePieces = state.occupancies[White];

    // ------------------ Generate the moves -------------------------------------
    U64 singlePushes = (blackPawns >> 8) & emptySquares;
    U64 doublePushes = (singlePushes >> 8) & emptySquares & rank5;

    // Captures (excludes the edges so pawns dont teleport to the other side of the board)
    U64 attacksLeft = (blackPawns & ~fileA) >> 9 & whitePieces;
    U64 attacksRight = (blackPawns & ~fileH) >> 7 & whitePieces;

    // En Passant captures
    if (state.enPassantSquare != NoSquare) {
        U64 epMask = 1ULL << state.enPassantSquare;
        attacksLeft |= (blackPawns & ~fileA) >> 9 & epMask;
        attacksRight |= (blackPawns & ~fileH) >> 7 & epMask;
    }

    // ------------------ Pack the moves into the struct -------------------------------------
    // Single Pushes
    while (singlePushes) {
        int targetSquare = popLSB(singlePushes);
        int startSquare = targetSquare + 8;

        if ((1ULL >> targetSquare) & rank1) {
            moveList.push_back(Move{startSquare, targetSquare, BlackQueen});
            moveList.push_back(Move{startSquare, targetSquare, BlackRook});
            moveList.push_back(Move{startSquare, targetSquare, BlackBishop});
            moveList.push_back(Move{startSquare, targetSquare, BlackKnight});
        } else {
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }

    // Double Pushes
    while (doublePushes) {
        int targetSquare = popLSB(doublePushes);
        int startSquare = targetSquare + 16;
        moveList.push_back(Move{startSquare, targetSquare, NoPiece});
    }

    // Attacks Left
    while (attacksLeft) {
        int targetSquare = popLSB(attacksLeft);
        int startSquare = targetSquare + 9;

        if ((1ULL >> targetSquare) & rank1) {
            moveList.push_back(Move{startSquare, targetSquare, BlackQueen});
            moveList.push_back(Move{startSquare, targetSquare, BlackRook});
            moveList.push_back(Move{startSquare, targetSquare, BlackBishop});
            moveList.push_back(Move{startSquare, targetSquare, BlackKnight});
        } else {
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }

    // Attacks Right
    while (attacksRight) {
        int targetSquare = popLSB(attacksRight);
        int startSquare = targetSquare + 7;

        if ((1ULL >> targetSquare) & rank1) {
            moveList.push_back(Move{startSquare, targetSquare, BlackQueen});
            moveList.push_back(Move{startSquare, targetSquare, BlackRook});
            moveList.push_back(Move{startSquare, targetSquare, BlackBishop});
            moveList.push_back(Move{startSquare, targetSquare, BlackKnight});
        } else {
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }
}

void generateKnightMoves(const BoardState& state, std::vector<Move>& moveList) {
    int us = state.sideToMove;

    U64 knights = state.bitboards[(us == White) ? WhiteKnight : BlackKnight];
    U64 friendlyPieces = state.occupancies[us];

    while (knights) {
        int startSquare = popLSB(knights);

        U64 validAttacks = knightAttacks[startSquare] & ~friendlyPieces;
        while (validAttacks) {
            int targetSquare = popLSB(validAttacks);
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }
}

void generateKingMoves(const BoardState& state, std::vector<Move>& moveList) {
    int us = state.sideToMove;
    int kingPiece = (us == White) ? WhiteKing : BlackKing;
    U64 kingBoard = state.bitboards[kingPiece];
    if (!kingBoard) return;

    int startSquare = popLSB(kingBoard);
    U64 friendlyPieces = state.occupancies[us];

    U64 validMoves = kingAttacks[startSquare] & ~friendlyPieces;
    while (validMoves) {
        int targetSquare = popLSB(validMoves);
        moveList.push_back(Move{startSquare, targetSquare, NoPiece});
    }

    // Castling Logic
    U64 allPieces = state.occupancies[Both];
    int them = (us == White) ? Black : White;

    if (us == White) {
        // Cant Castle in Check
        if (!isSquareAttacked(e1, them, state)) {

            // White Kingside
            if (state.castlingRights & wk) {
                // Squares f1 and g1 must be EMPTY
                if (!getBit(allPieces, f1) && !getBit(allPieces, g1)) {
                    // Squares f1 and g1 must be SAFE
                    if (!isSquareAttacked(f1, them, state) && !isSquareAttacked(g1, them, state)){
                        moveList.push_back(Move{e1, g1, NoPiece});
                    }
                }
            }
            // White Queenside
            if (state.castlingRights & wq) {
                // Squares d1, c1, and b1 must be EMPTY
                if (!getBit(allPieces, d1) && !getBit(allPieces, c1) && !getBit(allPieces, b1)) {
                    // Squares d1 and c1 must be SAFE
                    if (!isSquareAttacked(d1, them, state) && !isSquareAttacked(c1, them, state)){
                        moveList.push_back(Move{e1, c1, NoPiece});
                    }
                }
            }
        }
    } else {
        // Cant Castle in Check
        if (!isSquareAttacked(e8, them, state)) {
            // Black Kingside
            if (state.castlingRights & bk) {
                // Squares f8 and g8 must be EMPY
                if (!getBit(allPieces, f8) && !getBit(allPieces, g8)) {
                    // Squares f8 and g8 must be SAFE
                    if (!isSquareAttacked(f8, them, state) && !isSquareAttacked(g8, them, state)){
                        moveList.push_back(Move{e8, g8, NoPiece});
                    }
                }
            }
            // Black Queenside
            if (state.castlingRights & bq) {
                // Squares d8, c8, and b8 must be EMPY
                if (!getBit(allPieces, d8) && !getBit(allPieces, c8) && !getBit(allPieces, b8)) {
                    // Squares d8 and c8 must be SAFE
                    if (!isSquareAttacked(d8, them, state) && !isSquareAttacked(c8, them, state)){
                        moveList.push_back(Move{e8, c8, NoPiece});
                    }
                }
            }
        }
    }
}

void generateSlidingMoves(const BoardState& state, std::vector<Move>& moveList) {
    int us   = state.sideToMove;

    U64 rooks   = state.bitboards[(us == White) ? WhiteRook : BlackRook];
    U64 bishops = state.bitboards[(us == White) ? WhiteBishop : BlackBishop];
    U64 queens   = state.bitboards[(us == White) ? WhiteQueen : BlackQueen];

    U64 friendlyPieces = state.occupancies[us];

    // Generate Rooks
    while (rooks) {
        int startSquare = popLSB(rooks);
        U64 attacks = getRookAttacks(startSquare, state.occupancies[Both]);

        attacks &= ~friendlyPieces;
        while (attacks) {
            int targetSquare = popLSB(attacks);
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }

    // Generate Bishops
    while (bishops) {
        int startSquare = popLSB(bishops);
        U64 attacks = getBishopAttacks(startSquare, state.occupancies[Both]);

        attacks &= ~friendlyPieces;

        while (attacks) {
            int targetSquare = popLSB(attacks);
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }

    // Generate Queens
    while (queens) {
        int startSquare = popLSB(queens);
        U64 attacks = getQueenAttacks(startSquare, state.occupancies[Both]);
        
        // Mask out friendly pieces
        attacks &= ~friendlyPieces;
        
        while (attacks) {
            int targetSquare = popLSB(attacks);
            moveList.push_back(Move{startSquare, targetSquare, NoPiece});
        }
    }
}

void generateAllMoves(const BoardState& state, std::vector<Move>& moveList) {
    moveList.reserve(256);

    if (state.sideToMove == White) generateWhitePawnMoves(state, moveList);
    else generateBlackPawnMoves(state, moveList);

    generateKnightMoves(state, moveList);
    generateKingMoves(state, moveList);
    generateSlidingMoves(state, moveList);
}
