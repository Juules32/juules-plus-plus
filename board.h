#pragma once
#include <string.h>
#include "shorthands.h"




#define copy_board()                                                        \
    U64 bitboards_copy[12], occupancies_copy[3];                            \
    int side_copy, en_passant_copy, castle_copy;                            \
    memcpy(bitboards_copy, board::bitboards, board::size_of_bitboards);                   \
    memcpy(occupancies_copy, board::occupancies, board::size_of_occupancies);             \
    side_copy = board::side, en_passant_copy = board::en_passant, castle_copy = board::castle;   \



#define take_back()                                                         \
    memcpy(board::bitboards, bitboards_copy, board::size_of_bitboards);                   \
    memcpy(board::occupancies, occupancies_copy, board::size_of_occupancies);             \
    board::side = side_copy, board::en_passant = en_passant_copy, board::castle = castle_copy;   \








namespace board {

    
    
    // piece bitboards
    extern U64 bitboards[12];
    const int size_of_bitboards = sizeof(bitboards);

    // occupancy bitboards
    extern U64 occupancies[3];
    const int size_of_occupancies = sizeof(occupancies);

    // side to move
    extern int side;

    // en_passant square
    extern int en_passant; 

    // castling rights
    extern int castle;


    
    void print_bitboard(U64 bitboard);

    void print_game();

    void parse_fen(char[]);
    
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


    
};







