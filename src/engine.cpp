#include "types.h"
#include "board.h"
#include "movegen.h"
#include "eval.h"
#include <vector>
#include <iostream>
#include <chrono>

// Constants for checkmate and infinity
const int INFINITY_SCORE = 50000;
const int MATE_SCORE = 49000;

int alphaBeta(BoardState& state, int depth, int alpha, int beta) {
    if (depth == 0) return eval(state);

    std::vector<Move> moves;
    generateAllMoves(state, moves);

    int legalMoves = 0;
    int bestScore = -INFINITY_SCORE;

    for (Move move : moves) {
        BoardState savedState = state;

        if (makeMove(state, move)) {
            int us = savedState.sideToMove;
            int them = state.sideToMove;
            int kingPiece = (us == White) ? WhiteKing : BlackKing;
            int kingSquare = __builtin_ctzll(state.bitboards[kingPiece]);

            if (!isSquareAttacked(kingSquare, them, state)) {
                legalMoves++;
                int score = -alphaBeta(state, depth - 1, -beta, -alpha);
                
                if (score > bestScore) {
                    bestScore = score;
                }
                
                if (score > alpha) {
                    alpha = score;
                }
                
                if (alpha >= beta) {
                    state = savedState; 
                    break; 
                }
            }
        }
        state = savedState;
    }
    if (legalMoves == 0) {
        int kingPiece = (state.sideToMove == White) ? WhiteKing : BlackKing;
        int kingSquare = __builtin_ctzll(state.bitboards[kingPiece]);
        int them = (state.sideToMove == White) ? Black : White;

        if (isSquareAttacked(kingSquare, them, state)) {
            return -MATE_SCORE; // We are in checkmate, worst possible score
        } else {
            return 0; // Stalemate is a draw (0 centipawns)
        }
    }
    return bestScore;
}


Move getBestMove(BoardState& state, int depth) {
    std::vector<Move> moves;
    generateAllMoves(state, moves);

    Move bestMove = Move{-1, -1, NoPiece};
    int bestScore = -INFINITY_SCORE;
    int alpha = -INFINITY_SCORE;
    int beta = INFINITY_SCORE;

    for (Move move : moves) {
        BoardState savedState = state;

        if (makeMove(state, move)) {
            int us = savedState.sideToMove;
            int them = state.sideToMove;
            int kingPiece = (us == White) ? WhiteKing : BlackKing;
            int kingSquare = __builtin_ctzll(state.bitboards[kingPiece]);

            if (!isSquareAttacked(kingSquare, them, state)) {
                int score = -alphaBeta(state, depth - 1, -beta, -alpha);

                if (score > bestScore) {
                    bestScore = score;
                    bestMove = move;
                }

                if (score > alpha) {
                    alpha = score;
                }
            }
        }
        state = savedState;
    }
    return bestMove;
}



// ---------- Debug / Testing --------------------------
U64 perft(BoardState& state, int depth) {
    // Base case: If we've reached the bottom of the tree, this is 1 valid position.
    if (depth == 0) return 1ULL;

    std::vector<Move> moves;
    generateAllMoves(state, moves);

    U64 nodes = 0;
    int us = state.sideToMove;

    for (Move move : moves) {
        BoardState savedState = state;
        
        if (makeMove(state, move)) {
            // After makeMove executes, it swaps the sideToMove. 
            // So state.sideToMove is now the OPPONENT's color.
            int them = state.sideToMove;
            int kingPiece = (us == White) ? WhiteKing : BlackKing;
            
            // Find our King's square index safely
            int kingSquare = __builtin_ctzll(state.bitboards[kingPiece]);

            // If the opponent is NOT attacking our king, the move is legal!
            if (!isSquareAttacked(kingSquare, them, state)) {
                nodes += perft(state, depth - 1);
            }
        }
        
        // Undo the move
        state = savedState;
    }

    return nodes;
}

void perftDivide(BoardState& state, int depth) {
    if (depth == 0) return;

    std::cout << "--- Perft Divide Depth " << depth << " ---" << std::endl;

    std::vector<Move> moves;
    generateAllMoves(state, moves);

    // DEBUG 1: Did we generate any pseudo-legal moves?
    std::cout << "DEBUG: Pseudo-legal moves generated: " << moves.size() << std::endl;

    U64 totalNodes = 0;
    int us = state.sideToMove;

    // Start the timer
    auto startTime = std::chrono::high_resolution_clock::now();

    for (Move move : moves) {
        BoardState savedState = state;
        
        if (makeMove(state, move)) {
            int them = state.sideToMove;
            int kingPiece = (us == White) ? WhiteKing : BlackKing;
            int kingSquare = __builtin_ctzll(state.bitboards[kingPiece]);

            if (!isSquareAttacked(kingSquare, them, state)) {
                // Count all nodes branching off this specific move
                U64 nodes = perft(state, depth - 1);
                
                // Print the UCI string of the move and its node count
                std::cout << move.toString() << ": " << nodes << std::endl;
                
                totalNodes += nodes;
            } else {
                // DEBUG 2: Did the attack function filter it?
                std::cout << "DEBUG: " << move.toString() << " filtered (King in check)" << std::endl;
            }
        } else {
            // DEBUG 3: Did makeMove reject it?
            std::cout << "DEBUG: makeMove failed for " << move.toString() << std::endl;
        }
        state = savedState;
    }

    // Stop the timer
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = endTime - startTime;

    std::cout << "\nTotal Nodes: " << totalNodes << std::endl;
    std::cout << "Time elapsed: " << elapsed.count() << " seconds" << std::endl;
    
    // Calculate Nodes Per Second (NPS)
    U64 nps = (elapsed.count() > 0) ? (U64)(totalNodes / elapsed.count()) : 0;
    std::cout << "NPS: " << nps << std::endl;
}
