#pragma once
#include <map>
#include <string>
#include <string.h>
#include <iostream>
#include <chrono>
#include <queue>
using std::cout;
using std::cin;
using std::endl;

// Defining custom type U64, consisting of 64 zeroes
#define U64 unsigned long long

// Names for white, black, rook and bishop
enum {white, black, both};
enum {rook, bishop};
enum {P, N, B, R, Q, K, p, n, b, r, q, k, no_piece};
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

static const int size_of_bitboards = 96;
static const int size_of_occupancies = 24;

namespace state {
    // Piece bitboards
    extern U64 bitboards[12];

    // Occupancy bitboards
    extern U64 occupancies[3];

    // Side to move
    extern int side;

    // En_passant square
    extern int en_passant;

    // Castling rights
    extern int castle;

    static void merge_occupancies() {
        state::occupancies[both] = (state::occupancies[white] | state::occupancies[black]);
    }

    static void populate_occupancies()
    {
        memset(state::occupancies, 0ULL, size_of_occupancies);

        for (int piece_type = P; piece_type <= K; piece_type++)
        {
            state::occupancies[white] |= state::bitboards[piece_type];
        }

        for (int piece_type = p; piece_type <= k; piece_type++)
        {
            state::occupancies[black] |= state::bitboards[piece_type];
        }

        merge_occupancies();
    }
}





// Bit manipulation macros, basically shorthands
#define is_occupied(bitboard, square) (bitboard & (1ULL << square))
#define get_bit(bitboard, square) ((bitboard & (1ULL << square)) ? 1 : 0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (is_occupied(bitboard, square) ? bitboard ^= (1ULL << square) : 0)



struct moves
{
    int array[256];
    int size;
};


/*
          binary move bits                               hexidecimal constants

    0000 0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0000 1111 0000 0000 0000 0000 0000    capture flag        0x100000
    0001 0000 0000 0000 0000 0000 0000    double push flag    0x200000
    0010 0100 0000 0000 0000 0000 0000    en passant flag     0x400000
    0100 1000 0000 0000 0000 0000 0000    castling flag       0x800000
111 1000 0000 0000 0000 0000 0000 0000
*/

// Macros to extract move information
#define get_source(move) (move & 0x3f)
#define get_target(move) ((move & 0xfc0) >> 6)
#define get_piece(move) ((move & 0xf000) >> 12)
#define get_promotion_piece_type(move) ((move & 0xf0000) >> 16)
#define get_captured_piece_type(move) ((move & 0xf00000) >> 20)
#define is_capture(move) (((move & 0xf00000) >> 20) != no_piece)
#define is_double_pawn_push(move) (move & 0x1000000)
#define is_en_passant(move) (move & 0x2000000)
#define is_castling(move) (move & 0x4000000)
#define get_castle(move) ((move & 0x78000000) >> 27)


// Macro to encode move
#define encode_move(source, target, piece, promoted, capture, double_pawn_push, en_passant, castling) \
    (source) |                                                                                        \
        (target << 6) |                                                                               \
        (piece << 12) |                                                                               \
        (promoted << 16) |                                                                            \
        (capture << 20) |                                                                             \
        (double_pawn_push << 24) |                                                                    \
        (en_passant << 25) |                                                                          \
        (castling << 26)

// Lookup-tables relating converting from and to number and square name
static const std::string index_to_square[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

// ASCII pieces
static const std::string ascii_pieces[] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"};

static std::map<char, int> char_pieces = {
    {'P', P},
    {'N', N},
    {'B', B},
    {'R', R},
    {'Q', Q},
    {'K', K},
    {'p', p},
    {'n', n},
    {'b', b},
    {'r', r},
    {'q', q},
    {'k', k}};

static std::map<char, int> promoted_pieces = {
    {Q, 'q'},
    {R, 'r'},
    {B, 'b'},
    {N, 'n'},
    {q, 'q'},
    {r, 'r'},
    {b, 'b'},
    {n, 'n'},
    {0, ' '}};

// FEN dedug positions
static std::string start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
static std::string pawns_position = "8/pppppppp/8/8/8/8/PPPPPPPP/8 w KQkq - 0 1 ";
static std::string tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
static std::string killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
static std::string cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - -";
static std::string rook_position = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
static std::string promotion_position = "4k3/1P4P1/8/8/8/8/1pp3p1/4K3 w - - 0 1";
static std::string checkmate_position = "rnbqkbnr/ppppp2p/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1";
static std::string empty_position = "8/k7/8/8/8/8/K7/8 w - - ";

// Tables for scoring piece placements
static const int P_score[64] = 
{
    190, 190, 190, 190, 190, 190, 190, 190, 
    130, 130, 130, 140, 140, 130, 130, 130,
    120, 120, 120, 130, 130, 130, 120, 120,
    110, 110, 110, 120, 120, 110, 110, 110,
    105, 105, 110, 120, 120, 105, 105, 105,
    100, 100, 100, 105, 105, 100, 100, 100, 
    100, 100, 100, 90,  90,  100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100
};

static const int N_score[64] = 
{
    295, 300, 300, 300, 300, 300, 300, 295, 
    295, 300, 300, 310, 310, 300, 300, 295,
    295, 305, 320, 320, 320, 320, 305, 295,
    295, 310, 320, 330, 330, 320, 310, 295,
    295, 310, 320, 330, 330, 320, 310, 295,
    295, 305, 320, 310, 310, 320, 305, 295,
    295, 300, 300, 300, 300, 300, 300, 295,
    295, 290, 300, 300, 300, 300, 290, 295
};

static const int B_score[64] = 
{
    320, 320, 320, 320, 320, 320, 320, 320, 
    320, 320, 320, 320, 320, 320, 320, 320,
    320, 320, 320, 330, 330, 320, 320, 320,
    320, 320, 330, 340, 340, 330, 320, 320,
    320, 320, 330, 340, 340, 330, 320, 320, 
    320, 330, 320, 320, 320, 320, 330, 320,
    320, 350, 320, 320, 320, 320, 350, 320,
    320, 320, 310, 320, 320, 310, 320, 320
};

static const int R_score[64] =
{
    550, 550, 550, 550, 550, 550, 550, 550, 
    550, 550, 550, 550, 550, 550, 550, 550,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 500, 520, 520, 500, 500, 500
};

static const int Q_score[64] = 
{
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000
};

static const int K_score[64] = 
{
    10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 
    10000, 10000, 10005, 10005, 10005, 10005, 10000, 10000,
    10000, 10005, 10005, 10010, 10010, 10005, 10005, 10000,
    10000, 10005, 10010, 10020, 10020, 10010, 10005, 10000,
    10000, 10005, 10010, 10020, 10020, 10010, 10005, 10000,
    10000, 10000, 10005, 10010, 10010, 10005, 10000, 10000,
    10000, 10005, 10005, 9995, 9995, 10000, 10005, 10000,
    10000, 10000, 10005, 10000, 9985, 10000, 10010, 10000
};

static const int p_score[64] = 
{
    -100, -100, -100, -100, -100, -100, -100, -100, 
    -100, -100, -100,  -90,  -90, -100, -100, -100,
    -100, -100, -100, -105, -105, -100, -100, -100,
    -105, -105, -110, -120, -120, -105, -105, -105,
    -110, -110, -110, -120, -120, -110, -110, -110,
    -120, -120, -120, -130, -130, -130, -120, -120,
    -130, -130, -130, -140, -140, -130, -130, -130,
    -190, -190, -190, -190, -190, -190, -190, -190
};

static const int n_score[64] = 
{
    -295, -290, -300, -300, -300, -300, -290, -295, 
    -295, -300, -300, -300, -300, -300, -300, -295,
    -295, -305, -320, -310, -310, -320, -305, -295,
    -295, -310, -320, -330, -330, -320, -310, -295,
    -295, -310, -320, -330, -330, -320, -310, -295,
    -295, -305, -320, -320, -320, -320, -305, -295,
    -295, -300, -300, -310, -310, -300, -300, -295,
    -295, -300, -300, -300, -300, -300, -300, -295
};

static const int b_score[64] = 
{
    -320, -320, -310, -320, -320, -310, -320, -320, 
    -320, -350, -320, -320, -320, -320, -350, -320,
    -320, -330, -320, -320, -320, -320, -330, -320,
    -320, -320, -330, -340, -340, -330, -320, -320,
    -320, -320, -330, -340, -340, -330, -320, -320,
    -320, -320, -320, -330, -330, -320, -320, -320,
    -320, -320, -320, -320, -320, -320, -320, -320,
    -320, -320, -320, -320, -320, -320, -320, -320
};

static const int r_score[64] = 
{
    -500, -500, -500, -520, -520, -500, -500, -500, 
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500, 
    -550, -550, -550, -550, -550, -550, -550, -550,
    -550, -550, -550, -550, -550, -550, -550, -550
};

static const int q_score[64] = 
{
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, 
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000
};

static const int k_score[64] = 
{
    -10000, -10000, -10005, -10000, -9985, -10000, -10010, -10000, 
    -10000, -10005, -10005, -9995, -9995, -10000, -10005, -10000,
    -10000, -10000, -10005, -10010, -10010, -10005, -10000, -10000,
    -10000, -10005, -10010, -10020, -10020, -10010, -10005, -10000,
    -10000, -10005, -10010, -10020, -10020, -10010, -10005, -10000,
    -10000, -10005, -10005, -10010, -10010, -10005, -10005, -10000,
    -10000, -10000, -10005, -10005, -10005, -10005, -10000, -10000,
    -10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000
};

static const int* piece_score[] = {
    P_score, N_score, B_score, R_score, Q_score, K_score,
    p_score, n_score, b_score, r_score, q_score, k_score
};

// Most valuable victim - least valuable attacker [attacker][victim]
static const int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

// Castling rights update constants
const int castling_rights[64] = {
    7, 15, 15, 15, 3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14};

// Init move sorting helper arrays
static int killer_moves[2][246];
static int history_moves[12][246];
static int pv_length[246];
static int pv_table[246][246];

// Constant for null-move pruning
static const int reduced_depth_factor = 2;

static int ply = 0;
static std::uint64_t nodes = 0;
