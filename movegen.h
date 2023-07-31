#pragma once
#include "shorthands.h"

// Macros to extract move information
#define get_source(move) (move & 0x3f)
#define get_target(move) ((move & 0xfc0) >> 6)
#define get_piece(move) ((move & 0xf000) >> 12)
#define get_promotion_piece(move) ((move & 0xf0000) >> 16)
#define is_capture(move) (move & 0x100000)
#define is_double_pawn_push(move) (move & 0x200000)
#define is_en_passant(move) (move & 0x400000)
#define is_castling(move) (move & 0x800000)

/*
          binary move bits                               hexidecimal constants

    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    en passant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/

// Macro to encode move
#define encode_move(source, target, piece, promoted, capture, double_pawn_push, en_passant, castling) \
    (source) |                                                                                        \
        (target << 6) |                                                                               \
        (piece << 12) |                                                                               \
        (promoted << 16) |                                                                            \
        (capture << 20) |                                                                             \
        (double_pawn_push << 21) |                                                                    \
        (en_passant << 22) |                                                                          \
        (castling << 23)

/*
    The movegen namespace contains useful functions and variables
    relating to the move generation proces
*/

namespace movegen
{

    // Initializing empty attack arrays for the different pieces (pawns is 2d to account for color)
    extern U64 pawn_attacks[2][64];
    extern U64 pawn_quiet_moves[2][64];
    extern U64 knight_attacks[64];
    extern U64 king_attacks[64];
    extern U64 rook_masks[64];
    extern U64 bishop_masks[64];
    extern U64 rook_attacks[64][4096];
    extern U64 bishop_attacks[64][512];

    // Amount of squares rook/bishop can move to from square index
    extern const int bishop_relevant_bits[];
    extern const int rook_relevant_bits[];

    // Pre-generated magic numbers for slider move indexing
    extern const U64 rook_magic_numbers[64];
    extern const U64 bishop_magic_numbers[64];

    // Castling rights update constants
    extern const int castling_rights[64];

    void init();

    // Counts the amount of bits (1's) in a given bitboard
    static inline int count_bits(U64 bitboard)
    {
        int count = 0;

        while (bitboard)
        {
            ++count;
            bitboard &= bitboard - 1;
        }

        return count;
    }

    // Returns the least significant first bit in a bitboard (going from the top left corner)
    // The bitboard returned consists of 1's until the first 1 in the given bitboard is found
    static inline int get_ls1b(U64 bitboard)
    {
        return (bitboard ? count_bits((bitboard & -bitboard) - 1) : -1);
    }

    // The following leaper piece attack functions simply return
    // the attacks from the corresponding attack array index
    static inline U64 get_pawn_attacks(int side, int square)
    {
        return pawn_attacks[side][square];
    }

    static inline U64 get_knight_attacks(int square)
    {
        return knight_attacks[square];
    }

    static inline U64 get_king_attacks(int square)
    {
        return king_attacks[square];
    }

    // The following slider piece attack functions return
    // the corresponding piece mask, using magic numbers along the way
    // to get relevant index for the piece attack array.
    static inline U64 get_bishop_attacks(int square, U64 occupancy)
    {
        // Filters out squares the piece cannot move to
        occupancy &= bishop_masks[square];

        // Occupancy becomes magic index for bishop attack retrieval by using corresponding magic number
        occupancy *= bishop_magic_numbers[square];
        occupancy >>= 64 - bishop_relevant_bits[square];

        return bishop_attacks[square][occupancy];
    }

    static inline U64 get_rook_attacks(int square, U64 occupancy)
    {
        occupancy &= rook_masks[square];
        occupancy *= rook_magic_numbers[square];
        occupancy >>= 64 - rook_relevant_bits[square];
        return rook_attacks[square][occupancy];
    }

    static inline U64 get_queen_attacks(int square, U64 occupancy)
    {
        // Simply combines possible rook and bishop moves
        return get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy);
    }
}