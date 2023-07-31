#pragma once
#include <map>
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
extern char empty_board[];
extern char start_position[];
extern char pawns_position[];
extern char tricky_position[];
extern char killer_position[];
extern char cmk_position[];
extern char rook_position[];
