#include "magics.h"

// Define the massive tables in memory
U64 rookAttackTable[64][4096];
U64 bishopAttackTable[64][512];
U64 rookMasks[64];
U64 bishopMasks[64];

// --- 1. Generate Masks (Exclude Outer Edges) ---
U64 maskRookAttacks(int square) {
    U64 attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;
    for (int i = r + 1; i <= 6; i++) attacks |= (1ULL << (i * 8 + f));
    for (int i = r - 1; i >= 1; i--) attacks |= (1ULL << (i * 8 + f));
    for (int i = f + 1; i <= 6; i++) attacks |= (1ULL << (r * 8 + i));
    for (int i = f - 1; i >= 1; i--) attacks |= (1ULL << (r * 8 + i));
    return attacks;
}

U64 maskBishopAttacks(int square) {
    U64 attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;
    for (int r1 = r + 1, f1 = f + 1; r1 <= 6 && f1 <= 6; r1++, f1++) attacks |= (1ULL << (r1 * 8 + f1));
    for (int r1 = r + 1, f1 = f - 1; r1 <= 6 && f1 >= 1; r1++, f1--) attacks |= (1ULL << (r1 * 8 + f1));
    for (int r1 = r - 1, f1 = f + 1; r1 >= 1 && f1 <= 6; r1--, f1++) attacks |= (1ULL << (r1 * 8 + f1));
    for (int r1 = r - 1, f1 = f - 1; r1 >= 1 && f1 >= 1; r1--, f1--) attacks |= (1ULL << (r1 * 8 + f1));
    return attacks;
}

// --- 2. Slow On-The-Fly Generation (For building the tables) ---
U64 rookAttacksOnTheFly(int square, U64 block) {
    U64 attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;
    for (int i = r + 1; i <= 7; i++) { attacks |= (1ULL << (i * 8 + f)); if (block & (1ULL << (i * 8 + f))) break; }
    for (int i = r - 1; i >= 0; i--) { attacks |= (1ULL << (i * 8 + f)); if (block & (1ULL << (i * 8 + f))) break; }
    for (int i = f + 1; i <= 7; i++) { attacks |= (1ULL << (r * 8 + i)); if (block & (1ULL << (r * 8 + i))) break; }
    for (int i = f - 1; i >= 0; i--) { attacks |= (1ULL << (r * 8 + i)); if (block & (1ULL << (r * 8 + i))) break; }
    return attacks;
}

U64 bishopAttacksOnTheFly(int square, U64 block) {
    U64 attacks = 0ULL;
    int r = square / 8;
    int f = square % 8;
    for (int r1 = r + 1, f1 = f + 1; r1 <= 7 && f1 <= 7; r1++, f1++) { attacks |= (1ULL << (r1 * 8 + f1)); if (block & (1ULL << (r1 * 8 + f1))) break; }
    for (int r1 = r + 1, f1 = f - 1; r1 <= 7 && f1 >= 0; r1++, f1--) { attacks |= (1ULL << (r1 * 8 + f1)); if (block & (1ULL << (r1 * 8 + f1))) break; }
    for (int r1 = r - 1, f1 = f + 1; r1 >= 0 && f1 <= 7; r1--, f1++) { attacks |= (1ULL << (r1 * 8 + f1)); if (block & (1ULL << (r1 * 8 + f1))) break; }
    for (int r1 = r - 1, f1 = f - 1; r1 >= 0 && f1 >= 0; r1--, f1--) { attacks |= (1ULL << (r1 * 8 + f1)); if (block & (1ULL << (r1 * 8 + f1))) break; }
    return attacks;
}

// --- 3. Permutation Builder ---
U64 setOccupancy(int index, int bitsInMask, U64 attackMask) {
    U64 occupancy = 0ULL;
    for (int i = 0; i < bitsInMask; i++) {
        int square = __builtin_ctzll(attackMask); // Get LSB
        attackMask &= attackMask - 1;             // Pop LSB

        if (index & (1 << i)) {
            occupancy |= (1ULL << square);
        }
    }
    return occupancy;
}

// --- 4. The Main Initialization Routine ---
void initMagicBitboards() {
    for (int square = 0; square < 64; square++) {
        // Initialize Masks
        rookMasks[square] = maskRookAttacks(square);
        bishopMasks[square] = maskBishopAttacks(square);

        // Initialize Rook Table
        int rookBits = rookRelevantBits[square];
        int rookPermutations = 1 << rookBits;
        for (int index = 0; index < rookPermutations; index++) {
            U64 occupancy = setOccupancy(index, rookBits, rookMasks[square]);
            int magicIndex = (occupancy * rookMagics[square]) >> (64 - rookBits);
            rookAttackTable[square][magicIndex] = rookAttacksOnTheFly(square, occupancy);
        }

        // Initialize Bishop Table
        int bishopBits = bishopRelevantBits[square];
        int bishopPermutations = 1 << bishopBits;
        for (int index = 0; index < bishopPermutations; index++) {
            U64 occupancy = setOccupancy(index, bishopBits, bishopMasks[square]);
            int magicIndex = (occupancy * bishopMagics[square]) >> (64 - bishopBits);
            bishopAttackTable[square][magicIndex] = bishopAttacksOnTheFly(square, occupancy);
        }
    }
}