#pragma once
#include <string.h>
#include "shorthands.h"
#include "move_generation.h"

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



static inline int make_move(int move, bool only_captures = false) {
    if(only_captures) {
        if(is_capture(move)) {
            make_move(move, false);
        }
        else return 0;
    }

    else {
        //reset the en passant square
        en_passant = no_sq;

        copy_board();

        int source = get_source(move);
        int target = get_target(move);
        int piece = get_piece(move);
        int promotion_piece = get_promotion_piece(move);
        int capture = is_capture(move);
        int double_pawn_push = is_double_pawn_push(move);
        int en_pass = is_en_passant(move);
        int castling = is_castling(move);


        //move piece
        pop_bit(bitboards[piece], source);
        set_bit(bitboards[promotion_piece ? promotion_piece : piece], target);
        
        if(en_pass) {
            if(side == white) {
                pop_bit(bitboards[p], target + 8);
            }
            else {
                pop_bit(bitboards[P], target - 8);
            }
        }

        //if move is a capture, remove the attacked piece
        //important else, which saves running time
        else if(capture) {
            int start_piece, end_piece;
            if(side == white) {
                start_piece = p;
                end_piece = k;
            }
            else {
                start_piece = P;
                end_piece = K;
            }

            for (int bb_piece = start_piece; bb_piece < end_piece; bb_piece++)
            {
                if(is_occupied(bitboards[bb_piece], target)) {

                    pop_bit(bitboards[bb_piece], target);

                    break;
                }
            }
            
        }

        //set en passant square if a double pawn push was made
        //else is used to save time
        else if(double_pawn_push) {
            side == white ? en_passant = target + 8 :
                            en_passant = target - 8;
        }

        else if(castling) {
            switch (target) {
                case g1:
                    //if king side
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    break;
                
                case c1:
                    //if queen side
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    break;
                
                case g8:
                    //if king side
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    break;
                
                case c8:
                    //if queen side
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    break;
            }
        }

        //update castling rights
        castle &= castling_rights[source];
        castle &= castling_rights[target];

        //update occupancies
        update_occupancies();


        //check that the king is not in check
        if(is_square_attacked((side == white) ? get_ls1b(bitboards[K]) : get_ls1b(bitboards[k]), side ^ 1, bitboards, occupancies)) {
            take_back();
            //return illegal move
            return 0;
        }

        //switch sides
        side ^= 1;

        //return LEGAL move
        return 1;
    }
}