#pragma once
#include "shorthands.h"
#include "utils.h"
#include "movegen.h"

// Used to find out if a given square is attacked
static inline int is_square_attacked(int square, int side)
{

    // The key point is that any piece (with the exception of pawns) can reach the same square it moved from
    // A pawn of opposite color should overlap with one of the white pawns' attacks
    if (side == white && (movegen::get_pawn_attacks(black, square) & state::bitboards[P]))
        return 1;
    if (side == black && (movegen::get_pawn_attacks(white, square) & state::bitboards[p]))
        return 1;
    if (movegen::get_knight_moves(square) & ((side == white) ? state::bitboards[N] : state::bitboards[n]))
        return 1;
    if (movegen::get_king_moves(square) & ((side == white) ? state::bitboards[K] : state::bitboards[k]))
        return 1;

    // Sliders rely on current occupancy
    if (movegen::get_bishop_attacks(square, state::occupancies[both]) & ((side == white) ? state::bitboards[B] : state::bitboards[b]))
        return 1;
    if (movegen::get_rook_attacks(square, state::occupancies[both]) & ((side == white) ? state::bitboards[R] : state::bitboards[r]))
        return 1;
    if (movegen::get_queen_attacks(square, state::occupancies[both]) & ((side == white) ? state::bitboards[Q] : state::bitboards[q]))
        return 1;

    return 0;
}

// Macros for copying and reversing the current board state
#define copy_state(move) \
    int move_copy = (move | (state::castle << 27));

#define revert_state() \
    undo_move(move_copy);


// Used to make a move on the board
static inline void undo_move(int move)
{
    state::side ^= 1;

    int source = get_source(move);
    int target = get_target(move);
    int piece = get_piece(move);
    int promotion_piece_type = get_promotion_piece_type(move);
    int captured_piece = get_captured_piece_type(move);

    // Move piece
    pop_bit(state::bitboards[promotion_piece_type ? promotion_piece_type : piece], target);
    pop_bit(state::occupancies[state::side], target);
    set_bit(state::bitboards[piece], source);
    set_bit(state::occupancies[state::side], source);

    // If the move is en passant, remove the en passant-ed piece
    if (is_en_passant(move))
    {
        state::en_passant = target;

        if (state::side == white)
        {
            set_bit(state::bitboards[p], target + 8);
            set_bit(state::occupancies[black], target + 8);
        }
        else
        {
            set_bit(state::bitboards[P], target - 8);
            set_bit(state::occupancies[white], target - 8);
        }
    }

    // If move is a capture, remove the attacked piece
    // Important else, which saves running time
    else if (is_capture(move))
    {

        set_bit(state::bitboards[captured_piece], target);
        set_bit(state::occupancies[state::side ^ 1], target);
    }

    else if (is_castling(move))
    {
        switch (target)
        {
        case g1:
            // If king side
            set_bit(state::bitboards[R], h1);
            set_bit(state::occupancies[white], h1);
            pop_bit(state::bitboards[R], f1);
            pop_bit(state::occupancies[white], f1);
            break;

        case c1:
            // If queen side
            set_bit(state::bitboards[R], a1);
            set_bit(state::occupancies[white], a1);
            pop_bit(state::bitboards[R], d1);
            pop_bit(state::occupancies[white], d1);
            break;

        case g8:
            // If king side
            set_bit(state::bitboards[r], h8);
            set_bit(state::occupancies[black], h8);
            pop_bit(state::bitboards[r], f8);
            pop_bit(state::occupancies[black], f8);
            break;

        case c8:
            // If queen side
            set_bit(state::bitboards[r], a8);
            set_bit(state::occupancies[black], a8);
            pop_bit(state::bitboards[r], d8);
            pop_bit(state::occupancies[black], d8);
            break;
        }
    }

    state::castle = get_castle(move);

    // Update occupancies
    state::merge_occupancies();
}

/*
    The board namespace contains the board state
    As well as relevant functions to generate moves or move
*/
namespace board
{

    // Used to add a move to a move list
    static inline void add_move(moves *move_list, int move)
    {
        move_list->array[move_list->size] = move;
        ++move_list->size;
    }

    

    static int piece_array[64];

    // Used to generate all possible moves
    static inline void generate_moves(moves *move_list)
    {

        // Reset move count
        move_list->size = 0;

        // Define source & target squares
        int source_square, target_square;

        // Define current piece's bitboard copy and its attacks
        U64 bitboard, attacks;

        // Generate white pawns & white king castling moves
        if (state::side == white)
        {
            bitboard = state::bitboards[P];

            while (bitboard)
            {
                source_square = movegen::get_ls1b(bitboard);
                target_square = source_square - 8;

                // Generate quiet pawn moves
                if (!get_bit(state::occupancies[both], target_square))
                {
                    // Pawn promotion
                    if (source_square >= a7 && source_square <= h7)
                    {
                        add_move(move_list, encode_move(source_square, target_square, P, Q, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, R, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, B, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, N, no_piece, 0, 0, 0));
                    }

                    else
                    {
                        // One square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, P, 0, no_piece, 0, 0, 0));

                        // Two squares ahead pawn move
                        if ((source_square >= a2 && source_square <= h2) && !get_bit(state::occupancies[both], target_square - 8))
                            add_move(move_list, encode_move(source_square, target_square - 8, P, 0, no_piece, 1, 0, 0));
                    }
                }

                // Init pawn attacks bitboard
                attacks = movegen::get_pawn_attacks(state::side, source_square) & state::occupancies[black];

                while (attacks)
                {
                    target_square = movegen::get_ls1b(attacks);
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++)
                    {
                        if (is_occupied(state::bitboards[piece_type], target_square))
                        {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // Pawn promotion
                    if (source_square >= a7 && source_square <= h7)
                    {

                        add_move(move_list, encode_move(source_square, target_square, P, Q, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, R, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, B, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, N, target_piece, 0, 0, 0));
                    }

                    else
                        add_move(move_list, encode_move(source_square, target_square, P, 0, target_piece, 0, 0, 0));

                    // Pop ls1b of the pawn attacks
                    pop_bit(attacks, target_square);
                }

                // Generate en_passant captures
                if (state::en_passant != no_sq)
                {
                    // Lookup pawn attacks and bitwise AND with en_passant square (bit)
                    U64 en_passant_attacks = movegen::get_pawn_attacks(state::side, source_square) & (1ULL << state::en_passant);

                    // Make sure en_passant capture is available
                    if (en_passant_attacks)
                    {
                        // Init en_passant capture target square
                        int target_en_passant = movegen::get_ls1b(en_passant_attacks);
                        add_move(move_list, encode_move(source_square, target_en_passant, P, 0, p, 0, 1, 0));
                    }
                }

                // Pop ls1b from piece bitboard copy
                pop_bit(bitboard, source_square);
            }

            // castling moves
            bitboard = state::bitboards[K];

            // king side castling is available
            if (state::castle & wk)
            {
                // make sure square between king and king's rook are empty
                if (!get_bit(state::occupancies[both], f1) && !get_bit(state::occupancies[both], g1))
                {
                    // make sure king and the f1 squares are not under attacks
                    if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                        add_move(move_list, encode_move(e1, g1, K, 0, no_piece, 0, 0, 1));
                }
            }

            // queen side castling is available
            if (state::castle & wq)
            {
                // make sure square between king and queen's rook are empty
                if (!get_bit(state::occupancies[both], d1) && !get_bit(state::occupancies[both], c1) && !get_bit(state::occupancies[both], b1))
                {
                    // make sure king and the d1 squares are not under attacks
                    if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                        add_move(move_list, encode_move(e1, c1, K, 0, no_piece, 0, 0, 1));
                }
            }
        }

        // generate black pawns & black king castling moves
        else
        {
            bitboard = state::bitboards[p];

            // loop over white pawns within white pawn bitboard
            while (bitboard)
            {
                // init source square
                source_square = movegen::get_ls1b(bitboard);

                // init target square
                target_square = source_square + 8;

                // generate quiet pawn moves
                if (!get_bit(state::occupancies[both], target_square))
                {
                    // pawn promotion
                    if (source_square >= a2 && source_square <= h2)
                    {
                        add_move(move_list, encode_move(source_square, target_square, p, q, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, r, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, b, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, n, no_piece, 0, 0, 0));
                    }

                    else
                    {
                        // one square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, p, 0, no_piece, 0, 0, 0));

                        // two squares ahead pawn move
                        if ((source_square >= a7 && source_square <= h7) && !get_bit(state::occupancies[both], target_square + 8))
                            add_move(move_list, encode_move(source_square, target_square + 8, p, 0, no_piece, 1, 0, 0));
                    }
                }

                // init pawn attacks bitboard
                attacks = movegen::get_pawn_attacks(state::side, source_square) & state::occupancies[white];

                // generate pawn captures
                while (attacks)
                {
                    // init target square
                    target_square = movegen::get_ls1b(attacks);
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++)
                    {
                        if (is_occupied(state::bitboards[piece_type], target_square))
                        {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // pawn promotion
                    if (source_square >= a2 && source_square <= h2)
                    {
                        add_move(move_list, encode_move(source_square, target_square, p, q, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, r, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, b, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, n, target_piece, 0, 0, 0));
                    }

                    else
                        // one square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, p, 0, target_piece, 0, 0, 0));

                    // pop ls1b of the pawn attacks
                    pop_bit(attacks, target_square);
                }

                // generate en_passant captures
                if (state::en_passant != no_sq)
                {
                    // lookup pawn attacks and bitwise AND with en_passant square (bit)
                    U64 en_passant_attacks = movegen::get_pawn_attacks(state::side, source_square) & (1ULL << state::en_passant);

                    // make sure en_passant capture available
                    if (en_passant_attacks)
                    {
                        // init en_passant capture target square
                        int target_en_passant = movegen::get_ls1b(en_passant_attacks);
                        add_move(move_list, encode_move(source_square, target_en_passant, p, 0, P, 0, 1, 0));
                    }
                }

                // pop ls1b from piece bitboard copy
                pop_bit(bitboard, source_square);
            }

            bitboard = state::bitboards[k];

            // king side castling is available
            if (state::castle & bk)
            {
                // make sure square between king and king's rook are empty
                if (!get_bit(state::occupancies[both], f8) && !get_bit(state::occupancies[both], g8))
                {
                    // make sure king and the f8 squares are not under attacks
                    if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                        add_move(move_list, encode_move(e8, g8, k, 0, no_piece, 0, 0, 1));
                }
            }

            // queen side castling is available
            if (state::castle & bq)
            {
                // make sure square between king and queen's rook are empty
                if (!get_bit(state::occupancies[both], d8) && !get_bit(state::occupancies[both], c8) && !get_bit(state::occupancies[both], b8))
                {
                    // make sure king and the d8 squares are not under attacks
                    if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                        add_move(move_list, encode_move(e8, c8, k, 0, no_piece, 0, 0, 1));
                }
            }
        }

        int piece = (state::side == white ? N : n);
        bitboard = state::bitboards[piece];

        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_knight_moves(source_square) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));

                else
                {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++)
                    {
                        if (is_occupied(state::bitboards[piece_type], target_square))
                        {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate bishop moves
        piece = (state::side == white ? B : b);
        bitboard = state::bitboards[piece];

        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_bishop_attacks(source_square, state::occupancies[both]) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));

                else
                {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++)
                    {
                        if (is_occupied(state::bitboards[piece_type], target_square))
                        {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate rook moves
        piece = (state::side == white ? R : r);
        bitboard = state::bitboards[piece];
        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_rook_attacks(source_square, state::occupancies[both]) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));

                else
                {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++)
                    {
                        if (is_occupied(state::bitboards[piece_type], target_square))
                        {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate queen moves
        piece = (state::side == white ? Q : q);
        bitboard = state::bitboards[piece];
        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_queen_attacks(source_square, state::occupancies[both]) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));

                else
                {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++)
                    {
                        if (is_occupied(state::bitboards[piece_type], target_square))
                        {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate king moves
        piece = (state::side == white ? K : k);
        bitboard = state::bitboards[piece];
        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_king_moves(source_square) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));

                else
                {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white)
                    {
                        start_piece = p;
                        end_piece = k;
                    }
                    else
                    {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++)
                    {
                        if (is_occupied(state::bitboards[piece_type], target_square))
                        {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }
    }

    static inline int score_move(int move)
    {

        // Initialized to P since en passant capture
        // doesn't capture on target square
        int target_piece = P;

        // Score capture move
        if (is_capture(move))
        {

            return mvv_lva[get_piece(move)][get_captured_piece_type(move)] + 10000;
        }

        // Score quiet move
        else
        {

            // If move is a first priority killer move
            if (move == killer_moves[0][ply])
            {
                return 9000;
            }

            else if (move == killer_moves[1][ply])
            {
                return 8000;
            }

            else
            {
                return history_moves[get_piece(move)][get_target(move)];
            }
        }
    }

    static inline void sort_moves(moves *move_list)
    {

        int move_scores[move_list->size];

        for (int i = 0; i < move_list->size; i++)
        {
            move_scores[i] = score_move(move_list->array[i]);
        }

        for (int current_move = 0; current_move < move_list->size; current_move++)
        {
            for (int next_move = current_move + 1; next_move < move_list->size; next_move++)
            {
                if (move_scores[next_move] > move_scores[current_move])
                {
                    // swap scores
                    int temp_score = move_scores[current_move];
                    move_scores[current_move] = move_scores[next_move];
                    move_scores[next_move] = temp_score;

                    // swap moves
                    int temp_move = move_list->array[current_move];
                    move_list->array[current_move] = move_list->array[next_move];
                    move_list->array[next_move] = temp_move;
                }
            }
        }
    }

    // Used to make a move on the board
    static inline int make_move(int move)
    {

        // Reset the en passant square
        state::en_passant = no_sq;

        int source = get_source(move);
        int target = get_target(move);
        int piece = get_piece(move);
        int promotion_piece_type = get_promotion_piece_type(move);

        copy_state(move);

        // Move piece
        pop_bit(state::bitboards[piece], source);
        pop_bit(state::occupancies[state::side], source);
        set_bit(state::bitboards[promotion_piece_type ? promotion_piece_type : piece], target);
        set_bit(state::occupancies[state::side], target);

        // If the move is en passant, remove the en passant-ed piece
        if (is_en_passant(move))
        {
            if (state::side == white)
            {
                pop_bit(state::bitboards[p], target + 8);
                pop_bit(state::occupancies[black], target + 8);
            }
            else
            {
                pop_bit(state::bitboards[P], target - 8);
                pop_bit(state::occupancies[white], target - 8);
            }
        }

        // If move is a capture, remove the attacked piece
        // Important else, which saves running time
        else if (is_capture(move))
        {

            pop_bit(state::bitboards[get_captured_piece_type(move)], target);
            pop_bit(state::occupancies[state::side ^ 1], target);
        }

        // Set en passant square if a double pawn push was made
        // Else is used to save time
        else if (is_double_pawn_push(move))
        {
            state::side == white ? state::en_passant = target + 8 : state::en_passant = target - 8;
        }

        else if (is_castling(move))
        {
            switch (target)
            {
            case g1:
                // If king side
                pop_bit(state::bitboards[R], h1);
                pop_bit(state::occupancies[white], h1);
                set_bit(state::bitboards[R], f1);
                set_bit(state::occupancies[white], f1);
                break;

            case c1:
                // If queen side
                pop_bit(state::bitboards[R], a1);
                pop_bit(state::occupancies[white], a1);
                set_bit(state::bitboards[R], d1);
                set_bit(state::occupancies[white], d1);
                break;

            case g8:
                // If king side
                pop_bit(state::bitboards[r], h8);
                pop_bit(state::occupancies[black], h8);
                set_bit(state::bitboards[r], f8);
                set_bit(state::occupancies[black], f8);
                break;

            case c8:
                // If queen side
                pop_bit(state::bitboards[r], a8);
                pop_bit(state::occupancies[black], a8);
                set_bit(state::bitboards[r], d8);
                set_bit(state::occupancies[black], d8);
                break;
            }
        }

        // Update castling rights
        state::castle &= castling_rights[source];
        state::castle &= castling_rights[target];

        // Update occupancies
        state::merge_occupancies();

        // Otherwise, switch sides and return legal move
        state::side ^= 1;
        // Check that the king is not in check
        if (is_square_attacked((state::side == black) ? movegen::get_ls1b(state::bitboards[K]) : movegen::get_ls1b(state::bitboards[k]), state::side))
        {
            // If it is, revert back and return illegal move
            revert_state();
            return 0;
        }

        return 1;
    }

    static inline int eval()
    {
        U64 bitboard_copy;
        int square;
        int score = 0;

        // White pieces
        for (int piece_type = P; piece_type <= k; piece_type++)
        {
            bitboard_copy = state::bitboards[piece_type];

            while (bitboard_copy)
            {
                square = movegen::get_ls1b(bitboard_copy);
                pop_bit(bitboard_copy, square);
                score += piece_score[piece_type][square];
            }
        }

        return (state::side == white ? score : -score);
    }

    static inline int quiescence(int alpha, int beta)
    {
        // Check if command should terminate based on time spend calculating
        if (!(nodes % 4000))
        {
            check_if_time_is_up();
        }

        ++nodes;

        int evaluation = eval();

        if (evaluation >= beta)
        {
            return beta;
        }

        if (evaluation > alpha)
        {
            alpha = evaluation;
        }

        moves move_list[1];
        generate_moves(move_list);
        sort_moves(move_list);

        for (int i = 0; i < move_list->size; i++)
        {
            int current_move = move_list->array[i];

            copy_state(current_move);

            ++ply;

            // If move is illegal or not a capture
            if (!is_capture(move_list->array[i]) || !make_move(move_list->array[i]))
            {
                --ply;
                continue;
            }

            // Recursively determines if capture chain is beneficial
            int score = -quiescence(-beta, -alpha);
            --ply;

            revert_state();

            if (stop_calculating)
                return 0;

            if (score >= beta)
            {
                return beta;
            }

            if (score > alpha)
            {
                alpha = score;
            }
        }

        return alpha;
    }

    static inline int negamax(int alpha, int beta, int depth)
    {
        // Check if command should terminate based on time spend calculating

        pv_length[ply] = ply;

        if (!depth)
        {
            return quiescence(alpha, beta);
        }

        ++nodes;

        // Note whether king is currently in check
        int in_check = is_square_attacked(
            (state::side == white ? movegen::get_ls1b(state::bitboards[K]) : movegen::get_ls1b(state::bitboards[k])),
            state::side ^ 1);

        if (in_check)
            ++depth;

        // Null-move pruning
        // https://web.archive.org/web/20071031095933/http://www.brucemo.com/compchess/programming/nullmove.htm
        if (depth >= 3 && !in_check && ply)
        {
            int side_copy = state::side;
            int en_passant_copy = state::en_passant;

            // Imitates board as if it is opponent to move
            state::side ^= 1;
            state::en_passant = no_sq;

            int score = -negamax(-beta, -beta + 1, depth - 1 - reduced_depth_factor);

            state::side = side_copy;
            state::en_passant = en_passant_copy;

            if (stop_calculating)
                return 0;

            if (score >= beta)
                return beta;
        }

        // Keep track of the amount of legal moves
        int legal_moves = 0;

        // Move list init and find all moves
        moves move_list[1];
        generate_moves(move_list);
        sort_moves(move_list);

        for (int i = 0; i < move_list->size; i++)
        {
            int current_move = move_list->array[i];

            copy_state(current_move);

            ++ply;

            // If move is illegal
            if (!make_move(current_move))
            {
                --ply;
                continue;
            }

            // Else
            ++legal_moves;

            // Update score recursively with the negamax property:
            // https://www.chessprogramming.org/Negamax
            int score = -negamax(-beta, -alpha, depth - 1);
            --ply;

            revert_state();

            if (stop_calculating)
                return 0;

            // If a new, better move has been found
            if (score >= beta)
            {

                if (!is_capture(current_move))
                {
                    // Stores killer move for current ply
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = current_move;
                }

                return beta;
            }

            if (score > alpha)
            {

                // Stores history move depending on piece type and target square.
                // The idea is to improve score slightly every time a specific
                // move is seen to improve the position.
                // Higher depths are rewarded more, since deeper calculation is
                // generally more correct.
                if (!is_capture(current_move))
                {
                    history_moves[get_piece(current_move)][get_target(current_move)] += depth;
                }

                alpha = score;

                pv_table[ply][ply] = current_move;

                for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++)
                {
                    pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
                }

                pv_length[ply] = pv_length[ply + 1];
            }
        }

        // If there are no legal moves
        if (!legal_moves)
        {
            // King is in check (checkmate)
            if (in_check)
            {
                // "+ ply" prioritizes shorter checkmates
                return -49000 + ply;
            }

            // King is not in check (stalemate)
            else
            {
                return 0;
            }
        }

        return alpha;
    }

    void search_position(int depth);

}