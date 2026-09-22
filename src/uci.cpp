#include <iostream>
#include <string>
#include <sstream>
#include "board.h"
#include "types.h"
#include "movegen.h"
#include "engine.h"



Move parseMove(const std::string& moveStr, const BoardState& state) {
    // Safety check. Make sure the string is at least 4 char's
    if (moveStr.length() < 4) return Move{-1, -1, NoPiece};
    // Convert UCI string to square indices (0-63)
    // 'a' is ASCII 97. If we subtract 'a' from 'e', we get 4 (the E file).
    // '1' is ASCII 49. If we subtract '1' from '4', we get 3 (the 4th rank).
    int sourceFile = moveStr[0] - 'a';
    int sourceRank = moveStr[1] - '1';
    int targetFile = moveStr[2] - 'a';
    int targetRank = moveStr[3] - '1';

    int sourceSquare = sourceRank * 8 + sourceFile;
    int targetSquare = targetRank * 8 + targetFile;

    // Handle Promotions (Length 5 strings, e.g., "e7e8q")
    int promotedPiece = NoPiece;
    if (moveStr.length() == 5) {
        int us = state.sideToMove;
        char promChar = moveStr[4];
        
        if (promChar == 'q') promotedPiece = (us == White) ? WhiteQueen : BlackQueen;
        else if (promChar == 'r') promotedPiece = (us == White) ? WhiteRook : BlackRook;
        else if (promChar == 'b') promotedPiece = (us == White) ? WhiteBishop : BlackBishop;
        else if (promChar == 'n') promotedPiece = (us == White) ? WhiteKnight : BlackKnight;
    }

    // Verify against our move generator
    std::vector<Move> moveList;
    generateAllMoves(state, moveList);

    for (Move move : moveList) {
        // If the start and target squares match
        if (move.startSquare == sourceSquare && move.targetSquare == targetSquare) {
            
            // If it's a promotion, make sure the requested piece matches
            if (promotedPiece != NoPiece) {
                if (move.promotionPiece == promotedPiece) {
                    return move;
                }
                continue; // Wrong promotion piece, keep looking
            }
            
            // Return the fully validated move.
            return move; 
        }
    }

    // If we reach here, the GUI sent an illegal move.
    // Return a dummy move so the engine knows to reject it.
    return Move{-1, -1, NoPiece}; 
}


void parsePosition(const std::string& commandLine, BoardState& currentState) {
    std::istringstream stream(commandLine);
    std::string token;
    stream >> token; // Skip the word "position"

    stream >> token;
    if (token == "startpos") {
        initStartingPosition(currentState);
        stream >> token; // Move to the next token, which might be "moves"
    } 
    else if (token == "fen") {
        std::string fenString = "";
        // Collect the 6 parts of the FEN string
        for (int i = 0; i < 6 && stream >> token; i++) {
            fenString += token + " ";
        }
        parseFen(fenString, currentState);
        stream >> token; // Move to the next token, which might be "moves"
    }

    // If the GUI sent a list of moves made after the starting position/FEN
    if (token == "moves") {
        std::string moveString;
        while (stream >> moveString) {
            // Convert "e2e4" into internal Move struct, and makeMove() plays it on the board.
            Move parsedMove = parseMove(moveString, currentState);
            if (parsedMove.startSquare != -1) {
                makeMove(currentState, parsedMove);
            } else {
                std::cout << "Invalid move: " << moveString << std::endl;
            }
        }
    }
}

void parseGo(const std::string& commandLine, BoardState& currentState) {
    std::istringstream stream(commandLine);
    std::string token;
    stream >> token; // Skip the word "go"

    int depth = -1;
    int moveTime = -1;

    // Parse the parameters sent by the GUI
    while (stream >> token) {
        if (token == "depth") {
            stream >> depth;
        } 
        else if (token == "movetime") {
            stream >> moveTime;
        }
        // You can add wtime, btime, winc, binc here later!
    }

    // Default to depth 5 if the GUI didn't specify anything (like if you just typed "go")
    if (depth == -1) {
        depth = 5; 
    }

    // Note: To handle 'movetime' or tournament time, you will need Iterative Deepening.
    // For now, we will just use the depth variable.
    
    Move bestMove = getBestMove(currentState, depth);
    std::cout << "bestmove " << bestMove.toString() << std::endl;
}

// The main loop that listens to the GUI
void uciLoop(BoardState& currentState) {
    std::string commandLine;

    while (std::getline(std::cin, commandLine)) {
        std::istringstream stream(commandLine);
        std::string command;
        stream >> command;
        
        if (command == "uci") {
            //std::cout << "id name MyEngine\n";
            //std::cout << "id author You\n";
            std::cout << "uciok\n";
        }
        else if (command == "isready") {
            // The GUI sends this to make sure we aren't frozen
            std::cout << "readyok\n";
        } 
        else if (command == "ucinewgame") {
            // Reset everything for a fresh game
            initStartingPosition(currentState);
        } 
        else if (command == "position") {
            // Update our internal board state based on the GUI's instructions
            parsePosition(commandLine, currentState);
        } 
        else if (command == "go") {
            // The GUI is telling us to think and make a move.
            parseGo(commandLine, currentState);
        } 
        else if (command == "quit") {
            // Shut down the engine
            break;
        }
    }
}