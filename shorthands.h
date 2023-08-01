#pragma once
#include <map>
#include <string.h>
#include <iostream>
#include <chrono>
using namespace std;

// Defining custom type U64, consisting of 64 zeroes
#define U64 unsigned long long

// Bit manipulation macros, basically shorthands
#define is_occupied(bitboard, square) (bitboard & (1ULL << square))
#define get_bit(bitboard, square) ((bitboard & (1ULL << square)) ? 1 : 0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (is_occupied(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

// Names for white, black, rook and bishop
enum {white, black, both};
enum {rook, bishop};
enum {P, N, B, R, Q, K, p, n, b, r, q, k};
enum {wk = 1, wq = 2, bk = 4, bq = 8};

// Defining constant names to refer to corresponding index
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

// Lookup-tables relating converting from and to number and square name
extern const char *index_to_square[64];
extern const char ascii_pieces[13];
extern const char *unicode_pieces[12];
extern map<char, int> char_pieces;
extern map<char, int> promoted_pieces;

// FEN dedug positions
extern string empty_board;
extern string start_position;
extern string pawns_position;
extern string tricky_position;
extern string killer_position;
extern string cmk_position;
extern string rook_position;

struct moves
{
    int array[256];
    int size;
};

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

extern const int P_score[64];
extern const int N_score[64];
extern const int B_score[64];
extern const int R_score[64];
extern const int Q_score[64];
extern const int K_score[64];
extern const int p_score[64];
extern const int n_score[64];
extern const int b_score[64];
extern const int r_score[64];
extern const int q_score[64];
extern const int k_score[64];

extern const int* piece_score[64];