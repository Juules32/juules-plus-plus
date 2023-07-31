#pragma once
#include <string.h>
#include "shorthands.h"
#include "movegen.h"

#define copy_board()                                                          \
    U64 bitboards_copy[12], occupancies_copy[3];                              \
    int side_copy, en_passant_copy, castle_copy;                              \
    memcpy(bitboards_copy, board::bitboards, board::size_of_bitboards);       \
    memcpy(occupancies_copy, board::occupancies, board::size_of_occupancies); \
    side_copy = board::side, en_passant_copy = board::en_passant, castle_copy = board::castle;

#define revert_board()                                                        \
    memcpy(board::bitboards, bitboards_copy, board::size_of_bitboards);       \
    memcpy(board::occupancies, occupancies_copy, board::size_of_occupancies); \
    board::side = side_copy, board::en_passant = en_passant_copy, board::castle = castle_copy;

struct moves
{
    int array[256];
    int size;
};

namespace board
{

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

    static inline void add_move(moves *move_list, int move)
    {
        move_list->array[move_list->size] = move;
        ++move_list->size;
    }

    void parse_fen(char[]);

    static inline void update_occupancies()
    {
        memset(occupancies, 0ULL, size_of_occupancies);

        for (int piece_type = P; piece_type <= K; piece_type++)
        {
            occupancies[white] |= bitboards[piece_type];
        }

        for (int piece_type = p; piece_type <= k; piece_type++)
        {
            occupancies[black] |= bitboards[piece_type];
        }

        occupancies[both] |= occupancies[white];
        occupancies[both] |= occupancies[black];
    }

    static inline int is_square_attacked(int square, int side)
    {

        // The key point is that any piece (with the exception of pawns) can reach the same square it moved from

        // A pawn of opposite color should overlap with one of the white pawns' attacks
        if (side == white && (movegen::pawn_attacks[black][square] & bitboards[P]))
            return 1;
        if (side == black && (movegen::pawn_attacks[white][square] & bitboards[p]))
            return 1;
        if (movegen::knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n]))
            return 1;
        if (movegen::king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k]))
            return 1;

        // Sliders rely on current occupancy
        if (movegen::get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b]))
            return 1;
        if (movegen::get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r]))
            return 1;
        if (movegen::get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q]))
            return 1;

        return 0;
    }

    // generate all moves
    static inline void generate_moves(moves *move_list)
    {
        // init move count
        move_list->size = 0;

        // define source & target squares
        int source_square, target_square;

        // define current piece's bitboard copy & it's attacks
        U64 bitboard, attacks;

        // generate white pawns & white king castling moves
        if (side == white)
        {
            // pick up white pawn bitboards index
            // init piece bitboard copy
            bitboard = bitboards[P];

            // loop over white pawns within white pawn bitboard
            while (bitboard)
            {
                // init source square
                source_square = movegen::get_ls1b(bitboard);

                // init target square
                target_square = source_square - 8;

                // generate quiet pawn moves
                if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
                {
                    // pawn promotion
                    if (source_square >= a7 && source_square <= h7)
                    {
                        add_move(move_list, encode_move(source_square, target_square, P, Q, 0, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, R, 0, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, B, 0, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, N, 0, 0, 0, 0));
                    }

                    else
                    {
                        // one square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, P, 0, 0, 0, 0, 0));

                        // two squares ahead pawn move
                        if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                            add_move(move_list, encode_move(source_square, target_square - 8, P, 0, 0, 1, 0, 0));
                    }
                }

                // init pawn attacks bitboard
                attacks = movegen::pawn_attacks[side][source_square] & occupancies[black];

                // generate pawn captures
                while (attacks)
                {
                    // init target square
                    target_square = movegen::get_ls1b(attacks);

                    // pawn promotion
                    if (source_square >= a7 && source_square <= h7)
                    {
                        add_move(move_list, encode_move(source_square, target_square, P, Q, 1, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, R, 1, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, B, 1, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, N, 1, 0, 0, 0));
                    }

                    else
                        // one square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, P, 0, 1, 0, 0, 0));

                    // pop ls1b of the pawn attacks
                    pop_bit(attacks, target_square);
                }

                // generate en_passant captures
                if (en_passant != no_sq)
                {
                    // lookup pawn attacks and bitwise AND with en_passant square (bit)
                    U64 en_passant_attacks = movegen::pawn_attacks[side][source_square] & (1ULL << en_passant);

                    // make sure en_passant capture available
                    if (en_passant_attacks)
                    {
                        // init en_passant capture target square
                        int target_en_passant = movegen::get_ls1b(en_passant_attacks);
                        add_move(move_list, encode_move(source_square, target_en_passant, P, 0, 1, 0, 1, 0));
                    }
                }

                // pop ls1b from piece bitboard copy
                pop_bit(bitboard, source_square);
            }

            // castling moves
            bitboard = bitboards[K];

            // king side castling is available
            if (castle & wk)
            {
                // make sure square between king and king's rook are empty
                if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                {
                    // make sure king and the f1 squares are not under attacks
                    if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                        add_move(move_list, encode_move(e1, g1, K, 0, 0, 0, 0, 1));
                }
            }

            // queen side castling is available
            if (castle & wq)
            {
                // make sure square between king and queen's rook are empty
                if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
                {
                    // make sure king and the d1 squares are not under attacks
                    if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                        add_move(move_list, encode_move(e1, c1, K, 0, 0, 0, 0, 1));
                }
            }
        }

        // generate black pawns & black king castling moves
        else
        {
            bitboard = bitboards[p];

            // loop over white pawns within white pawn bitboard
            while (bitboard)
            {
                // init source square
                source_square = movegen::get_ls1b(bitboard);

                // init target square
                target_square = source_square + 8;

                // generate quiet pawn moves
                if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
                {
                    // pawn promotion
                    if (source_square >= a2 && source_square <= h2)
                    {
                        add_move(move_list, encode_move(source_square, target_square, p, q, 0, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, r, 0, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, b, 0, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, n, 0, 0, 0, 0));
                    }

                    else
                    {
                        // one square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, p, 0, 0, 0, 0, 0));

                        // two squares ahead pawn move
                        if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                            add_move(move_list, encode_move(source_square, target_square + 8, p, 0, 0, 1, 0, 0));
                    }
                }

                // init pawn attacks bitboard
                attacks = movegen::pawn_attacks[side][source_square] & occupancies[white];

                // generate pawn captures
                while (attacks)
                {
                    // init target square
                    target_square = movegen::get_ls1b(attacks);

                    // pawn promotion
                    if (source_square >= a2 && source_square <= h2)
                    {
                        add_move(move_list, encode_move(source_square, target_square, p, q, 1, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, r, 1, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, b, 1, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, n, 1, 0, 0, 0));
                    }

                    else
                        // one square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, p, 0, 1, 0, 0, 0));

                    // pop ls1b of the pawn attacks
                    pop_bit(attacks, target_square);
                }

                // generate en_passant captures
                if (en_passant != no_sq)
                {
                    // lookup pawn attacks and bitwise AND with en_passant square (bit)
                    U64 en_passant_attacks = movegen::pawn_attacks[side][source_square] & (1ULL << en_passant);

                    // make sure en_passant capture available
                    if (en_passant_attacks)
                    {
                        // init en_passant capture target square
                        int target_en_passant = movegen::get_ls1b(en_passant_attacks);
                        add_move(move_list, encode_move(source_square, target_en_passant, p, 0, 1, 0, 1, 0));
                    }
                }

                // pop ls1b from piece bitboard copy
                pop_bit(bitboard, source_square);
            }

            bitboard = bitboards[k];

            // king side castling is available
            if (castle & bk)
            {
                // make sure square between king and king's rook are empty
                if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                {
                    // make sure king and the f8 squares are not under attacks
                    if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                        add_move(move_list, encode_move(e8, g8, k, 0, 0, 0, 0, 1));
                }
            }

            // queen side castling is available
            if (castle & bq)
            {
                // make sure square between king and queen's rook are empty
                if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                {
                    // make sure king and the d8 squares are not under attacks
                    if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                        add_move(move_list, encode_move(e8, c8, k, 0, 0, 0, 0, 1));
                }
            }
        }

        int piece = (side == white ? N : n);
        bitboard = bitboards[piece];

        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                else
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate bishop moves
        piece = (side == white ? B : b);
        bitboard = bitboards[piece];

        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                else
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate rook moves
        piece = (side == white ? R : r);
        bitboard = bitboards[piece];
        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                else
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate queen moves
        piece = (side == white ? Q : q);
        bitboard = bitboards[piece];
        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                else
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }

        // generate king moves
        piece = (side == white ? K : k);
        bitboard = bitboards[piece];
        // loop over source squares of piece bitboard copy
        while (bitboard)
        {
            // init source square
            source_square = movegen::get_ls1b(bitboard);

            // init piece attacks in order to get set of target squares
            attacks = movegen::king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);

            // loop over target squares available from generated attacks
            while (attacks)
            {
                // init target square
                target_square = movegen::get_ls1b(attacks);

                // quiet move
                if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));

                else
                    // capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));

                // pop ls1b in current attacks set
                pop_bit(attacks, target_square);
            }

            // pop ls1b of the current piece bitboard copy
            pop_bit(bitboard, source_square);
        }
    }

    static inline int make_move(int move, bool only_captures = false)
    {
        if (only_captures)
        {
            if (is_capture(move))
            {
                make_move(move, false);
            }
            else
                return 0;
        }

        else
        {
            // reset the en passant square
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

            // move piece
            pop_bit(bitboards[piece], source);
            set_bit(bitboards[promotion_piece ? promotion_piece : piece], target);

            if (en_pass)
            {
                if (side == white)
                {
                    pop_bit(bitboards[p], target + 8);
                }
                else
                {
                    pop_bit(bitboards[P], target - 8);
                }
            }

            // if move is a capture, remove the attacked piece
            // important else, which saves running time
            else if (capture)
            {
                int start_piece, end_piece;
                if (side == white)
                {
                    start_piece = p;
                    end_piece = k;
                }
                else
                {
                    start_piece = P;
                    end_piece = K;
                }

                for (int bb_piece = start_piece; bb_piece < end_piece; bb_piece++)
                {
                    if (is_occupied(bitboards[bb_piece], target))
                    {

                        pop_bit(bitboards[bb_piece], target);

                        break;
                    }
                }
            }

            // set en passant square if a double pawn push was made
            // else is used to save time
            else if (double_pawn_push)
            {
                side == white ? en_passant = target + 8 : en_passant = target - 8;
            }

            else if (castling)
            {
                switch (target)
                {
                case g1:
                    // if king side
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    break;

                case c1:
                    // if queen side
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    break;

                case g8:
                    // if king side
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    break;

                case c8:
                    // if queen side
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    break;
                }
            }

            // update castling rights
            castle &= movegen::castling_rights[source];
            castle &= movegen::castling_rights[target];

            // update occupancies
            update_occupancies();

            // check that the king is not in check
            if (is_square_attacked((side == white) ? movegen::get_ls1b(bitboards[K]) : movegen::get_ls1b(bitboards[k]), side ^ 1))
            {
                revert_board();
                // return illegal move
                return 0;
            }

            // switch sides
            side ^= 1;

            // return LEGAL move
            return 1;
        }
    }

};
