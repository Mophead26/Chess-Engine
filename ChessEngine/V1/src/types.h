#pragma once

#include <cstdint>
#include <string>


const int pawnVal = 100;
const int knightVal = 350;
const int bishopVal = 365;
const int rookVal = 525;
const int queenVal = 1000;
const int kingVal = 20000;



typedef uint64_t U64;


inline void setBit(U64& bitboard, int square) {
    bitboard |= (1ULL << square);
}

inline void clearBit(U64& bitboard, int square) {
    bitboard &= ~(1ULL << square);
}

inline bool getBit(U64 bitboard, int square) {
    return (bitboard & (1ULL << square)) != 0;
}

const int directionOffsets[] = {8, -8, 1, -1, 7, -7, 9, -9};

// Bitboard Masks
const U64 rank1    = 0x00000000000000FFULL;
const U64 rank2    = 0x000000000000FF00ULL;
const U64 rank3    = 0x0000000000FF0000ULL;
const U64 rank4    = 0x00000000FF000000ULL;
const U64 rank5    = 0x000000FF00000000ULL;
const U64 rank6    = 0x0000FF0000000000ULL;
const U64 rank7    = 0x00FF000000000000ULL;
const U64 rank8    = 0xFF00000000000000ULL;

const U64 fileA    = 0x0101010101010101ULL;
const U64 fileB    = 0x0202020202020202ULL;
const U64 fileC    = 0x0404040404040404ULL;
const U64 fileD    = 0x0808080808080808ULL;
const U64 fileE    = 0x1010101010101010ULL;
const U64 fileF    = 0x2020202020202020ULL;
const U64 fileG    = 0x4040404040404040ULL;
const U64 fileH    = 0x8080808080808080ULL;

// Lookup tables for moves
inline U64 knightAttacks[64];
inline U64 kingAttacks[64]; 


enum Color {
    White = 0,
    Black = 1,
    Both = 2
};

enum Piece {
    WhitePawn = 0, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing,
    BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing,
    NoPiece
};

enum Square {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8,
    NoSquare
};

enum CastlingRights {
    wk = 1, // White Kingside
    wq = 2, // White Queenside
    bk = 4, // Black Kingside
    bq = 8  // Black Queenside
};

struct BoardState {

    U64 bitboards[12];      // Piece Bitboards

    U64 occupancies[3];     // WhitePiece, BlackPiece, and AllPiece Bitboards

    int sideToMove;         // White (0), Black (1)

    int enPassantSquare;    // Target square for enPassant capture, set to NoSquare if none

    uint8_t castlingRights; // 4 bit int to track who can castle where (wk, wq, bk, bq)

    // Game Clocks
    int halfMoveClock;   // 50 move rule
    int fullMoveNumber;  // increments when Black moves. Used for UCI and FEN string gen
};


// For a simple engine, keeping the start, target, and promotion piece 
// explicitly defined makes debugging much easier than packing them into a single int.
struct Move {
    int startSquare;
    int targetSquare;
    int promotionPiece; // Set to NoPiece if not a promotion

    // Converts our enum squares back into UCI string format (e.g., "e2e4")
    std::string toString() const {
        std::string moveString = "";
        
        // Start square
        moveString += char('a' + (startSquare % 8));
        moveString += char('1' + (startSquare / 8));
        
        // Target square
        moveString += char('a' + (targetSquare % 8));
        moveString += char('1' + (targetSquare / 8));
        
        // Promotion (UCI expects lowercase letters for promotion: q, r, b, n)
        if (promotionPiece != NoPiece) {
            if (promotionPiece == WhiteQueen || promotionPiece == BlackQueen) moveString += 'q';
            else if (promotionPiece == WhiteRook || promotionPiece == BlackRook) moveString += 'r';
            else if (promotionPiece == WhiteBishop || promotionPiece == BlackBishop) moveString += 'b';
            else if (promotionPiece == WhiteKnight || promotionPiece == BlackKnight) moveString += 'n';
        }
        
        return moveString;
    }
};