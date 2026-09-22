#include <iostream>
#include <cstring>
#include <sstream>
#include <cctype>
#include <cstring>
#include <algorithm>
#include "types.h"
#include "board.h"


void updateOccupancies(BoardState& state) {
    // Zero out the occupancies first
    state.occupancies[White] = 0ULL;
    state.occupancies[Black] = 0ULL;
    state.occupancies[Both]  = 0ULL;

    // OR together all White pieces (indices 0 to 5)
    for (int piece = WhitePawn; piece <= WhiteKing; piece++) {
        state.occupancies[White] |= state.bitboards[piece];
    }
    // OR together all Black pieces (indices 6 to 11)
    for (int piece = BlackPawn; piece <= BlackKing; piece++) {
        state.occupancies[Black] |= state.bitboards[piece];
    }

    // Combine them for the Both mask
    state.occupancies[Both] = state.occupancies[White] | state.occupancies[Black];
}

void initStartingPosition(BoardState& state) {
    // Wipe the entire struct to zero (clears all bitboards and variables)
    std::memset(&state, 0, sizeof(BoardState));

    // Set White Pieces
    state.bitboards[WhitePawn]   = 0x000000000000FF00ULL; // Rank 2
    state.bitboards[WhiteRook]   = 0x0000000000000081ULL; // b1, g1
    state.bitboards[WhiteKnight] = 0x0000000000000042ULL; // c1, f1
    state.bitboards[WhiteBishop] = 0x0000000000000024ULL; // a1, h1
    state.bitboards[WhiteQueen]  = 0x0000000000000008ULL; // d1
    state.bitboards[WhiteKing]   = 0x0000000000000010ULL; // e1

    // Set Black Pieces
    state.bitboards[BlackPawn]   = 0x00FF000000000000ULL; // Rank 7
    state.bitboards[BlackRook]   = 0x8100000000000000ULL; // b8, g8
    state.bitboards[BlackKnight] = 0x4200000000000000ULL; // c8, f8
    state.bitboards[BlackBishop] = 0x2400000000000000ULL; // a8, h8
    state.bitboards[BlackQueen]  = 0x0800000000000000ULL; // d8
    state.bitboards[BlackKing]   = 0x1000000000000000ULL; // e8

    // Set Game Metadata
    state.sideToMove = White;
    state.enPassantSquare = NoSquare; 
    state.castlingRights = wk | wq | bk | bq; // (1111 in binary)
    state.halfMoveClock = 0;
    state.fullMoveNumber = 1;

    // Generate the occupancy masks based on the bitboards we just set
    updateOccupancies(state);
}

void parseFen(const std::string& fenString, BoardState& state) {
    // 1. Clear the board completely
    std::memset(&state, 0, sizeof(BoardState));

    std::istringstream fenStream(fenString);
    std::string pieces, activeColor, castling, enPassant, halfMove, fullMove;

    // Split the FEN string by spaces into our 6 components
    fenStream >> pieces >> activeColor >> castling >> enPassant >> halfMove >> fullMove;

    // --- Part 1: Parse Pieces ---
    // FEN strings start at Rank 8 (index 7) and File A (index 0)
    int currentRank = 7; 
    int currentFile = 0; 

    for (char symbol : pieces) {
        if (symbol == '/') {
            currentRank--;     // Move down a rank
            currentFile = 0;   // Reset to File A
        } 
        else if (std::isdigit(symbol)) {
            // A number means empty squares. '3' means advance file by 3.
            currentFile += (symbol - '0'); 
        } 
        else {
            int pieceType = -1;
            switch (symbol) {
                case 'P': pieceType = WhitePawn; break;
                case 'N': pieceType = WhiteKnight; break;
                case 'B': pieceType = WhiteBishop; break;
                case 'R': pieceType = WhiteRook; break;
                case 'Q': pieceType = WhiteQueen; break;
                case 'K': pieceType = WhiteKing; break;
                case 'p': pieceType = BlackPawn; break;
                case 'n': pieceType = BlackKnight; break;
                case 'b': pieceType = BlackBishop; break;
                case 'r': pieceType = BlackRook; break;
                case 'q': pieceType = BlackQueen; break;
                case 'k': pieceType = BlackKing; break;
            }
            
            if (pieceType != -1) {
                int currentSquare = currentRank * 8 + currentFile;
                setBit(state.bitboards[pieceType], currentSquare);
                currentFile++;
            }
        }
    }

    // --- Part 2: Active Color ---
    state.sideToMove = (activeColor == "w") ? White : Black;

    // --- Part 3: Castling Rights ---
    state.castlingRights = 0;
    if (castling != "-") {
        for (char c : castling) {
            if (c == 'K') state.castlingRights |= wk;
            if (c == 'Q') state.castlingRights |= wq;
            if (c == 'k') state.castlingRights |= bk;
            if (c == 'q') state.castlingRights |= bq;
        }
    }

    // --- Part 4: En Passant Target ---
    if (enPassant != "-") {
        // Convert string like "e3" into a square index (0-63)
        int fileIndex = enPassant[0] - 'a'; // 'a' -> 0, 'e' -> 4
        int rankIndex = enPassant[1] - '1'; // '1' -> 0, '3' -> 2
        state.enPassantSquare = rankIndex * 8 + fileIndex;
    } else {
        state.enPassantSquare = NoSquare;
    }

    // --- Part 5 & 6: Clocks ---
    // We check if they are empty because some malformed FENs omit them
    if (!halfMove.empty()) state.halfMoveClock = std::stoi(halfMove);
    if (!fullMove.empty()) state.fullMoveNumber = std::stoi(fullMove);

    // --- Final Step ---
    // Regenerate the occupancy bitboards to match the new piece layout
    updateOccupancies(state);
}

void printBitboard(U64 bitboard) {
    std::cout << "\n";
    
    // Loop from Rank 8 (index 7) down to Rank 1 (index 0)
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << "  "; // Print rank labels on the left

        for (int file = 0; file <= 7; file++) {
            // Calculate the 0-63 square index
            int square = rank * 8 + file; 
            
            // Check if the bit at this square is 1
            if (bitboard & (1ULL << square)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
    
    // Print file labels at the bottom
    std::cout << "   a b c d e f g h\n\n";
    
    // Print the raw decimal/hex value (useful for copy-pasting bitboard masks)
    std::cout << "   Hex: 0x" << std::hex << bitboard << std::dec << "ULL\n\n";
}

void printBoard(const BoardState& state) {
    std::cout << "\n";

    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << "  "; 
        
        for (int file = 0; file <= 7; file++) {
            int square = rank * 8 + file;
            char pieceChar = '.'; // Default to empty square

            // Mask for the current square
            U64 squareMask = 1ULL << square;

            // Check White pieces
            if      (state.bitboards[WhitePawn]   & squareMask) pieceChar = 'P';
            else if (state.bitboards[WhiteKnight] & squareMask) pieceChar = 'N';
            else if (state.bitboards[WhiteBishop] & squareMask) pieceChar = 'B';
            else if (state.bitboards[WhiteRook]   & squareMask) pieceChar = 'R';
            else if (state.bitboards[WhiteQueen]  & squareMask) pieceChar = 'Q';
            else if (state.bitboards[WhiteKing]   & squareMask) pieceChar = 'K';
            
            // Check Black pieces
            else if (state.bitboards[BlackPawn]   & squareMask) pieceChar = 'p';
            else if (state.bitboards[BlackKnight] & squareMask) pieceChar = 'n';
            else if (state.bitboards[BlackBishop] & squareMask) pieceChar = 'b';
            else if (state.bitboards[BlackRook]   & squareMask) pieceChar = 'r';
            else if (state.bitboards[BlackQueen]  & squareMask) pieceChar = 'q';
            else if (state.bitboards[BlackKing]   & squareMask) pieceChar = 'k';

            std::cout << pieceChar << " ";
        }
        std::cout << "\n";
    }
    
    std::cout << "   a b c d e f g h\n\n";

    // --- Print Game Metadata ---
    std::cout << "Side to Move:    " << (state.sideToMove == White ? "White" : "Black") << "\n";

    // Print Castling Rights (e.g., KQkq or K-kq)
    std::cout << "Castling Rights: ";
    std::cout << ((state.castlingRights & wk) ? 'K' : '-');
    std::cout << ((state.castlingRights & wq) ? 'Q' : '-');
    std::cout << ((state.castlingRights & bk) ? 'k' : '-');
    std::cout << ((state.castlingRights & bq) ? 'q' : '-');
    std::cout << "\n";

    // Print En Passant Target Square
    std::cout << "En Passant:      ";
    if (state.enPassantSquare != NoSquare) {
        char fileChar = 'a' + (state.enPassantSquare % 8);
        char rankChar = '1' + (state.enPassantSquare / 8);
        std::cout << fileChar << rankChar << "\n";
    } else {
        std::cout << "None\n";
    }
    std::cout << "\n";
}

bool makeMove(BoardState& state, Move move) {
    int us = state.sideToMove;
    int them = (us == White) ? Black : White;

    int movingPiece = NoPiece;
    int capturedPiece = NoPiece;

    // Identify the moving Piece, we only check our pieces
    int startIdx = (us == White) ? WhitePawn : BlackPawn;
    int endIdx   = (us == White) ? WhiteKing : BlackKing;
    for (int piece = startIdx; piece <= endIdx; piece++) {
        if (getBit(state.bitboards[piece], move.startSquare)) {
            movingPiece = piece;
            break;
        }
    }
    if (movingPiece == NoPiece) {
        /*std::cout << "\n--- CRITICAL MAKEMOVE DEBUG ---\n";
        std::cout << "Move string: " << move.toString() << "\n";
        std::cout << "Raw Start Square Index: " << move.startSquare << "\n";
        std::cout << "Side to move: " << us << " (White=" << White << ", Black=" << Black << ")\n";
        std::cout << "Looping pieces from index " << startIdx << " to " << endIdx << "\n";
        
        // Let's force-check the WhitePawn bitboard for the very first move
        std::cout << "Is there a pawn on this square? " << getBit(state.bitboards[WhitePawn], move.startSquare) << "\n";
        std::cout << "-------------------------------\n";*/
        return false;
    }

    // Identify the captured piece if any
    int oppStateIdx = (them == White) ? WhitePawn : BlackPawn;
    int oppEndIdx   = (them == White) ? WhiteKing : BlackKing;
    for (int piece = oppStateIdx; piece <= oppEndIdx; piece++) {
        if(getBit(state.bitboards[piece], move.targetSquare)) {
            capturedPiece = piece;
            break;
        }
    }
    // Move the piece on the board
    clearBit(state.bitboards[movingPiece], move.startSquare);
    setBit(state.bitboards[movingPiece], move.targetSquare);

    // Handle Captures
    if (capturedPiece != NoPiece) {
        clearBit(state.bitboards[capturedPiece], move.targetSquare);
    }
    else if ((movingPiece == WhitePawn || movingPiece == BlackPawn) && move.targetSquare == state.enPassantSquare) {
        // En Passant Capture: Delete the pawn sitting one rank behind the target square
        if (us == White) {
            clearBit(state.bitboards[BlackPawn], move.targetSquare - 8);
        } else {
            clearBit(state.bitboards[WhitePawn], move.targetSquare + 8);
        }
    }

    // Handle Promotions
    if (move.promotionPiece != NoPiece) {
        // Remove the pawn from the target square
        clearBit(state.bitboards[movingPiece], move.targetSquare);
        // Place the new promoted piece (Queen, Rook, Bishop, or Knight)
        setBit(state.bitboards[move.promotionPiece], move.targetSquare);
    }

    // Castling
    if (movingPiece == WhiteKing || movingPiece == BlackKing) {
        // If the King moved 2 squares horizontally, it's a castle
        if (move.targetSquare - move.startSquare == 2) { 
            // Kingside: Move Rook from h-file to f-file
            if (us == White) { clearBit(state.bitboards[WhiteRook], h1); setBit(state.bitboards[WhiteRook], f1); }
            else             { clearBit(state.bitboards[BlackRook], h8); setBit(state.bitboards[BlackRook], f8); }
        } else if (move.targetSquare - move.startSquare == -2) {
            // Queenside: Move Rook from a-file to d-file
            if (us == White) { clearBit(state.bitboards[WhiteRook], a1); setBit(state.bitboards[WhiteRook], d1); }
            else             { clearBit(state.bitboards[BlackRook], a8); setBit(state.bitboards[BlackRook], d8); }
        }
    }

    // Update Clocks
    if (movingPiece == WhitePawn || movingPiece == BlackPawn || capturedPiece != NoPiece) {
        state.halfMoveClock = 0; // Reset 50-move rule on pawn push or capture
    } else {
        state.halfMoveClock++;
    }
    if (us == Black) {
        state.fullMoveNumber++; // Increment full move after Black plays
    }

    // Update Castling Rights
    if (movingPiece == WhiteKing) state.castlingRights &= ~(wk | wq);
    if (movingPiece == BlackKing) state.castlingRights &= ~(bk | bq);

    // If the corners are interacted with in any way revoke the right
    if (move.startSquare == h1 || move.targetSquare == h1) state.castlingRights &= ~wk;
    if (move.startSquare == a1 || move.targetSquare == a1) state.castlingRights &= ~wq;
    if (move.startSquare == h8 || move.targetSquare == h8) state.castlingRights &= ~bk;
    if (move.startSquare == a8 || move.targetSquare == a8) state.castlingRights &= ~bq;

    // Update En Passant Target Square
    state.enPassantSquare = NoSquare;
    if (movingPiece == WhitePawn && (move.targetSquare - move.startSquare == 16)) {
        state.enPassantSquare = move.startSquare + 8;
    }
    else if (movingPiece == BlackPawn && (move.startSquare - move.targetSquare == 16)) {
        state.enPassantSquare = move.startSquare - 8;
    }

    // Swap turns and update occupancies
    state.sideToMove = them;
    updateOccupancies(state);

    return true;
}

void initDistanceToEdge() {
    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 8; rank++) {
            
            // Convert file/rank back into the 0-63 square index
            int square = rank * 8 + file;
            
            // Calculate distance to the straight edges
            int numNorth = 7 - rank;
            int numSouth = rank;
            int numEast  = 7 - file;
            int numWest  = file;
            
            // Calculate distance to the diagonal edges
            // The distance diagonally is always the shortest distance of the two straight paths
            int numNW = std::min(numNorth, numWest);
            int numSE = std::min(numSouth, numEast);
            int numNE = std::min(numNorth, numEast);
            int numSW = std::min(numSouth, numWest);
            
            // Store the results
            numSquaresToEdge[square][0] = numNorth;
            numSquaresToEdge[square][1] = numSouth;
            numSquaresToEdge[square][2] = numEast;
            numSquaresToEdge[square][3] = numWest;
            numSquaresToEdge[square][4] = numNW;
            numSquaresToEdge[square][5] = numSE;
            numSquaresToEdge[square][6] = numNE;
            numSquaresToEdge[square][7] = numSW;
        }
    }
}

void initKnightMoves() {
    for (int square = 0; square < 64; square++) {
        U64 knight = 1ULL << square; // Place a knight on the current square
        U64 attacks = 0;

        // --- North Jumps (Left Shifts) ---
        // Up 2, Right 1 (+17). Prevent wrapping off the H-file.
        attacks |= (knight & ~fileH) << 17; 
        // Up 2, Left 1 (+15). Prevent wrapping off the A-file.
        attacks |= (knight & ~fileA) << 15;
        // Up 1, Right 2 (+10). Prevent wrapping off the G or H files.
        attacks |= (knight & (~fileH & ~fileG)) << 10;
        // Up 1, Left 2 (+6). Prevent wrapping off the A or B files.
        attacks |= (knight & (~fileA & ~fileB)) << 6;

        // --- South Jumps (Right Shifts) ---
        // Down 2, Right 1 (-15)
        attacks |= (knight & ~fileH) >> 15;
        // Down 2, Left 1 (-17)
        attacks |= (knight & ~fileA) >> 17;
        // Down 1, Right 2 (-6)
        attacks |= (knight & (~fileH & ~fileG)) >> 6;
        // Down 1, Left 2 (-10)
        attacks |= (knight & (~fileA & ~fileB)) >> 10;

        knightAttacks[square] = attacks;
    }
}

void initKingMoves() {
    for (int square = 0; square < 64; square++) {
        U64 king = 1ULL << square;
        U64 attacks = 0;

        attacks |= (king << 8);          // North
        attacks |= (king >> 8);          // South
        attacks |= (king & ~fileH) << 1; // East
        attacks |= (king & ~fileA) >> 1; // West

        attacks |= (king & ~fileH) << 9; // North-East
        attacks |= (king & ~fileA) << 7; // North-West
        attacks |= (king & ~fileH) >> 7; // South-East
        attacks |= (king & ~fileA) >> 9; // South-West
        
        kingAttacks[square] = attacks;
    }
}

bool isSquareAttacked(int square, int attackingColor, const BoardState& state) {
    // Enemy bitboards
    U64 pawns   = state.bitboards[(attackingColor == White) ? WhitePawn   : BlackPawn];
    U64 knights = state.bitboards[(attackingColor == White) ? WhiteKnight : BlackKnight];
    U64 bishops = state.bitboards[(attackingColor == White) ? WhiteBishop : BlackBishop];
    U64 rooks   = state.bitboards[(attackingColor == White) ? WhiteRook   : BlackRook];
    U64 queens  = state.bitboards[(attackingColor == White) ? WhiteQueen  : BlackQueen];
    U64 kings   = state.bitboards[(attackingColor == White) ? WhiteKing   : BlackKing];

    // Knights and King
    if (knightAttacks[square] & knights) return true;
    if (kingAttacks[square] & kings) return true;

    // Pawn Attacks
    U64 sqBB = 1ULL << square;
    if (attackingColor == White) {
        if (((sqBB & ~fileA) >> 9) & pawns) return true;
        if (((sqBB & ~fileH) >> 7) & pawns) return true;
    } else { // Attacking color is Black (they move down, so we look up)
        if (((sqBB & ~fileA) << 7) & pawns) return true;
        if (((sqBB & ~fileH) << 9) & pawns) return true;
    }

    // Sliding Attacks (Orthogonal)
    for (int dir = 0; dir < 4; dir++) {
        for (int n = 0; n < numSquaresToEdge[square][dir]; n++) {
            int target = square + directionOffsets[dir] * (n + 1);
            U64 targetMask = 1ULL << target;

            // If we hit ANY piece, the ray stops
            if (state.occupancies[Both] & targetMask) {
                // Check if the piece we hit is an enemy Rook or Queen
                if ((rooks | queens) & targetMask) {
                    return true;
                }
                break; // Hit a blocker (friendly or irrelevant enemy), stop this ray
            }
        }
    }

    // Sliding Attacks (Diagonal)
    for (int dir = 4; dir < 8; dir++) {
        for (int n = 0; n < numSquaresToEdge[square][dir]; n++) {
            int target = square + directionOffsets[dir] * (n + 1);
            U64 targetMask = 1ULL << target;

            // If we hit ANY piece, the ray stops
            if (state.occupancies[Both] & targetMask) {
                // Check if the piece we hit is an enemy Bishop or Queen
                if ((bishops | queens) & targetMask) {
                    return true;
                }
                break; 
            }
        }
    }

    // Square is safe
    return false;
}

