#pragma once
#define U64 unsigned long long
#include "shorthands.h"

// piece bitboards
extern U64 bitboards[12];
extern const int size_of_bitboards;

// occupancy bitboards
extern U64 occupancies[3];
extern const int size_of_occupancies;

// side to move
extern int side;

// en_passant square
extern int en_passant; 

// castling rights
extern int castle;

#define copy_board()                                                        \
    U64 bitboards_copy[12], occupancies_copy[3];                            \
    int side_copy, en_passant_copy, castle_copy;                            \
    memcpy(bitboards_copy, bitboards, size_of_bitboards);                   \
    memcpy(occupancies_copy, occupancies, size_of_occupancies);             \
    side_copy = side, en_passant_copy = en_passant, castle_copy = castle;   \

#define take_back()                                                         \
    memcpy(bitboards, bitboards_copy, size_of_bitboards);                   \
    memcpy(occupancies, occupancies_copy, size_of_occupancies);             \
    side = side_copy, en_passant = en_passant_copy, castle = castle_copy;   \

static inline void update_occupancies() {
    memset(occupancies, 0ULL, size_of_occupancies);

    for(int piece_type = P; piece_type <= K; piece_type++) {
        occupancies[white] |= bitboards[piece_type];
    }

    for(int piece_type = p; piece_type <= k; piece_type++) {
        occupancies[black] |= bitboards[piece_type];
    }

    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
}

void print_bitboard(U64 bitboard);

void print_game();

void parse_fen(char[]);
