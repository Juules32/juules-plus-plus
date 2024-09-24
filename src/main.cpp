/*

I know what you're thinking - one big file, really?

Yes. I've found it to be simpler this way;

- Faster compilation times

- No annoying header files with decentralized code

*/

#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <string.h>
#include <iostream>
#include <chrono>
#include <queue>
using std::cout;
using std::cin;
using std::endl;

// Custom type U64, consisting of 64 bits used for bitboards
typedef unsigned long long U64;

// Enums for white, black, rook and bishop
enum {white, black, both};
enum {rook, bishop};

// Piece types
enum {P, N, B, R, Q, K, p, n, b, r, q, k, no_piece};

// Enum used for encoding castling rights 
enum {wk = 1, wq = 2, bk = 4, bq = 8};

// Square indices
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

// Size, in bytes, of all piece bitboards
const int BITBOARDS_SIZE = 96;

// Size, in bytes, of all occupancy bitboards
const int OCCUPANCIES_SIZE = 24;

// Data structure containing a list of moves
struct moves {
    int array[256];
    int size;
};

// Bit manipulation macros, basically shorthands
#define is_occupied(bitboard, square) (bitboard & (1ULL << square))
#define get_bit(bitboard, square) ((bitboard & (1ULL << square)) ? 1 : 0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (is_occupied(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

/*
          Binary move bits                     Hexidecimal constants

    0000 0000 0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 0000 0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0000 0000 1111 0000 0000 0000 0000 0000    captured piece      0xf00000
    0000 1111 0000 0000 0000 0000 0000 0000    castling state      0xf000000
    0001 0000 0000 0000 0000 0000 0000 0000    double push flag    0x10000000
    0010 0000 0000 0000 0000 0000 0000 0000    en passant flag     0x20000000
    0100 0000 0000 0000 0000 0000 0000 0000    castling flag       0x40000000
*/

// Macros to extract move information
#define get_source(move) (move & 0x3f)                              // Source square of the move
#define get_target(move) ((move & 0xfc0) >> 6)                      // Target square of the move
#define get_piece(move) ((move & 0xf000) >> 12)                     // Piece type that moved
#define get_promotion_piece_type(move) ((move & 0xf0000) >> 16)     // Promotion piece type, in case pawn promoted
#define get_captured_piece_type(move) ((move & 0xf00000) >> 20)     // Piece type of the captured piece, if any
#define is_capture(move) (((move & 0xf00000) >> 20) != no_piece)    // Helper function returning whether move was a capture
#define get_castle(move) ((move & 0xf000000) >> 24)                 // Castling state before the move was played
#define is_double_pawn_push(move) (move & 0x10000000)               // Flag indicating whether move was a double pawn push
#define is_en_passant(move) (move & 0x20000000)                     // Flag indicating whether move was en passant
#define is_castling(move) (move & 0x40000000)                       // Flag indicating whether move was castling

// Macro to encode move
#define encode_move(source, target, piece, promoted, capture, double_pawn_push, en_passant, castling) \
    (source) |                                                                                        \
    (target << 6) |                                                                                   \
    (piece << 12) |                                                                                   \
    (promoted << 16) |                                                                                \
    (capture << 20) |                                                                                 \
    (double_pawn_push << 28) |                                                                        \
    (en_passant << 29) |                                                                              \
    (castling << 30)

// Lookup-tables relating converting from and to number and square name
const std::string index_to_square[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

// ASCII pieces
const std::string ascii_pieces[] = {"P", "N", "B", "R", "Q", "K", "p", "n", "b", "r", "q", "k"};

std::map<char, int> char_pieces = {
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
    {'k', k}
};

std::map<char, int> promoted_pieces = {
    {Q, 'q'},
    {R, 'r'},
    {B, 'b'},
    {N, 'n'},
    {q, 'q'},
    {r, 'r'},
    {b, 'b'},
    {n, 'n'},
    {0, ' '}
};

// Predefined FEN strings
std::string start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
std::string pawns_position = "8/pppppppp/8/8/8/8/PPPPPPPP/8 w KQkq - 0 1 ";
std::string tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 ";
std::string killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
std::string cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - -";
std::string rook_position = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
std::string promotion_position = "4k3/1P4P1/8/8/8/8/1pp3p1/4K3 w - - 0 1";
std::string checkmate_position = "rnbqkbnr/ppppp2p/8/8/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1";
std::string empty_position = "8/k7/8/8/8/8/K7/8 w - - ";

// Arrays for scoring piece placements
const int P_score[64] = {
    190, 190, 190, 190, 190, 190, 190, 190, 
    130, 130, 130, 140, 140, 130, 130, 130,
    120, 120, 120, 130, 130, 130, 120, 120,
    110, 110, 110, 120, 120, 110, 110, 110,
    105, 105, 110, 120, 120, 105, 105, 105,
    100, 100, 100, 105, 105, 100, 100, 100, 
    100, 100, 100, 90,  90,  100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100
};

const int N_score[64] = {
    295, 300, 300, 300, 300, 300, 300, 295, 
    295, 300, 300, 310, 310, 300, 300, 295,
    295, 305, 320, 320, 320, 320, 305, 295,
    295, 310, 320, 330, 330, 320, 310, 295,
    295, 310, 320, 330, 330, 320, 310, 295,
    295, 305, 320, 310, 310, 320, 305, 295,
    295, 300, 300, 300, 300, 300, 300, 295,
    295, 290, 300, 300, 300, 300, 290, 295
};

const int B_score[64] = {
    320, 320, 320, 320, 320, 320, 320, 320, 
    320, 320, 320, 320, 320, 320, 320, 320,
    320, 320, 320, 330, 330, 320, 320, 320,
    320, 320, 330, 340, 340, 330, 320, 320,
    320, 320, 330, 340, 340, 330, 320, 320, 
    320, 330, 320, 320, 320, 320, 330, 320,
    320, 350, 320, 320, 320, 320, 350, 320,
    320, 320, 310, 320, 320, 310, 320, 320
};

const int R_score[64] = {
    550, 550, 550, 550, 550, 550, 550, 550, 
    550, 550, 550, 550, 550, 550, 550, 550,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 510, 520, 520, 510, 500, 500,
    500, 500, 500, 520, 520, 500, 500, 500
};

const int Q_score[64] = {
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
    1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000
};

const int K_score[64] = {
    10000, 10000, 10000, 10000, 10000, 10000, 10000, 10000, 
    10000, 10000, 10005, 10005, 10005, 10005, 10000, 10000,
    10000, 10005, 10005, 10010, 10010, 10005, 10005, 10000,
    10000, 10005, 10010, 10020, 10020, 10010, 10005, 10000,
    10000, 10005, 10010, 10020, 10020, 10010, 10005, 10000,
    10000, 10000, 10005, 10010, 10010, 10005, 10000, 10000,
    10000, 10005, 10005, 9995, 9995, 10000, 10005, 10000,
    10000, 10000, 10005, 10000, 9985, 10000, 10010, 10000
};

const int p_score[64] = {
    -100, -100, -100, -100, -100, -100, -100, -100, 
    -100, -100, -100,  -90,  -90, -100, -100, -100,
    -100, -100, -100, -105, -105, -100, -100, -100,
    -105, -105, -110, -120, -120, -105, -105, -105,
    -110, -110, -110, -120, -120, -110, -110, -110,
    -120, -120, -120, -130, -130, -130, -120, -120,
    -130, -130, -130, -140, -140, -130, -130, -130,
    -190, -190, -190, -190, -190, -190, -190, -190
};

const int n_score[64] = {
    -295, -290, -300, -300, -300, -300, -290, -295, 
    -295, -300, -300, -300, -300, -300, -300, -295,
    -295, -305, -320, -310, -310, -320, -305, -295,
    -295, -310, -320, -330, -330, -320, -310, -295,
    -295, -310, -320, -330, -330, -320, -310, -295,
    -295, -305, -320, -320, -320, -320, -305, -295,
    -295, -300, -300, -310, -310, -300, -300, -295,
    -295, -300, -300, -300, -300, -300, -300, -295
};

const int b_score[64] = {
    -320, -320, -310, -320, -320, -310, -320, -320, 
    -320, -350, -320, -320, -320, -320, -350, -320,
    -320, -330, -320, -320, -320, -320, -330, -320,
    -320, -320, -330, -340, -340, -330, -320, -320,
    -320, -320, -330, -340, -340, -330, -320, -320,
    -320, -320, -320, -330, -330, -320, -320, -320,
    -320, -320, -320, -320, -320, -320, -320, -320,
    -320, -320, -320, -320, -320, -320, -320, -320
};

const int r_score[64] = {
    -500, -500, -500, -520, -520, -500, -500, -500, 
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500,
    -500, -500, -510, -520, -520, -510, -500, -500, 
    -550, -550, -550, -550, -550, -550, -550, -550,
    -550, -550, -550, -550, -550, -550, -550, -550
};

const int q_score[64] = {
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000, 
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000,
    -1000, -1000, -1000, -1000, -1000, -1000, -1000, -1000
};

const int k_score[64] = {
    -10000, -10000, -10005, -10000, -9985, -10000, -10010, -10000, 
    -10000, -10005, -10005, -9995, -9995, -10000, -10005, -10000,
    -10000, -10000, -10005, -10010, -10010, -10005, -10000, -10000,
    -10000, -10005, -10010, -10020, -10020, -10010, -10005, -10000,
    -10000, -10005, -10010, -10020, -10020, -10010, -10005, -10000,
    -10000, -10005, -10005, -10010, -10010, -10005, -10005, -10000,
    -10000, -10000, -10005, -10005, -10005, -10005, -10000, -10000,
    -10000, -10000, -10000, -10000, -10000, -10000, -10000, -10000
};

const int* piece_score[] = {
    P_score, N_score, B_score, R_score, Q_score, K_score,
    p_score, n_score, b_score, r_score, q_score, k_score
};

// Most valuable victim - least valuable attacker [attacker][victim]
const int mvv_lva[12][12] = {
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

// Class to measure time for various purposes
class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;

public:
    Timer() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void reset() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    double get_time_passed_millis() {
        auto current_time = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
    }
};

/*
    The format namespace contains functions related to formatting.
*/
namespace format {
    std::string eval(int eval) {
        double divided_score = static_cast<double>(eval) / 100.0;
        std::string formatted_score = std::to_string(divided_score);
        size_t dotPosition = formatted_score.find('.');
        
        // Ensure there are exactly two decimal places
        if (dotPosition != std::string::npos && dotPosition + 3 < formatted_score.length()) {
            formatted_score = formatted_score.substr(0, dotPosition + 3);
        }
        
        return formatted_score;
    }

    std::string move(int move) {
        return index_to_square[get_source(move)] +
            index_to_square[get_target(move)] + 
            char(promoted_pieces[get_promotion_piece_type(move)]);
    }
}

/*
    The rng namespace contains functions related to random number generation.
*/
namespace rng {
    unsigned int random_state = 1804289383;

    // Generates a random 32-bit number from the current state
    unsigned int generate_32_bit() {
        unsigned int number = random_state;

        number ^= number << 13;
        number ^= number >> 17;
        number ^= number << 5;

        // Updates state so new numbers can be found
        random_state = number;

        return number;
    }

    // Generates a random 64-bit number
    unsigned int generate_64_bit() {

        // Generates four different 32-bit numbers where the first 16 bits are 0
        U64 n1 = (U64)(generate_32_bit()) & 0xFFFF;
        U64 n2 = (U64)(generate_32_bit()) & 0xFFFF;
        U64 n3 = (U64)(generate_32_bit()) & 0xFFFF;
        U64 n4 = (U64)(generate_32_bit()) & 0xFFFF;

        // Slices them all together
        return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
    }
}

/*
    The rng namespace contains utility functions used in various places for various purposes.
*/
namespace util {
    // Counts the amount of bits (1's) in a given bitboard
    static inline int count_bits(U64 bitboard) {
        int count = 0;

        while (bitboard) {
            ++count;
            bitboard &= bitboard - 1;
        }

        return count;
    }

    // Returns the least significant first bit in a bitboard (going from the top left corner)
    // The bitboard returned consists of 1's until the first 1 in the given bitboard is found
    static inline int get_ls1b(U64 bitboard) {
        return (count_bits((bitboard & -bitboard) - 1));
    }
}

/*
    The state namespace contains all necessary information about the game state.
*/
namespace state {
    // Piece bitboards
    U64 bitboards[12];

    // Occupancy bitboards
    U64 occupancies[3];

    // Side to move
    int side = -1;

    // En passant square
    int en_passant = no_sq;

    // Castling rights
    int castle = 0;
}

/*
    The move_gen namespace contains useful functions and variables related to the move generation process.
    This includes initialization functions as well as functions for generating all available moves.
*/
namespace move_gen {
    // Empty attack arrays for the different pieces (pawns is 2d to account for color)
    U64 pawn_attacks[2][64];
    U64 pawn_quiet_moves[2][64];
    U64 knight_moves[64];
    U64 king_moves[64];
    U64 rook_masks[64];
    U64 bishop_masks[64];
    U64 rook_attacks[64][4096];
    U64 bishop_attacks[64][512];

    // Amount of legal moves bishop/rook can make from square index
    const int bishop_relevant_bits[] = {
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6};

    const int rook_relevant_bits[] = {
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12};

    // Pre-generated magic numbers for slider move indexing
    const U64 rook_magic_numbers[64] = {
        0x8a80104000800020ULL,
        0x140002000100040ULL,
        0x2801880a0017001ULL,
        0x100081001000420ULL,
        0x200020010080420ULL,
        0x3001c0002010008ULL,
        0x8480008002000100ULL,
        0x2080088004402900ULL,
        0x800098204000ULL,
        0x2024401000200040ULL,
        0x100802000801000ULL,
        0x120800800801000ULL,
        0x208808088000400ULL,
        0x2802200800400ULL,
        0x2200800100020080ULL,
        0x801000060821100ULL,
        0x80044006422000ULL,
        0x100808020004000ULL,
        0x12108a0010204200ULL,
        0x140848010000802ULL,
        0x481828014002800ULL,
        0x8094004002004100ULL,
        0x4010040010010802ULL,
        0x20008806104ULL,
        0x100400080208000ULL,
        0x2040002120081000ULL,
        0x21200680100081ULL,
        0x20100080080080ULL,
        0x2000a00200410ULL,
        0x20080800400ULL,
        0x80088400100102ULL,
        0x80004600042881ULL,
        0x4040008040800020ULL,
        0x440003000200801ULL,
        0x4200011004500ULL,
        0x188020010100100ULL,
        0x14800401802800ULL,
        0x2080040080800200ULL,
        0x124080204001001ULL,
        0x200046502000484ULL,
        0x480400080088020ULL,
        0x1000422010034000ULL,
        0x30200100110040ULL,
        0x100021010009ULL,
        0x2002080100110004ULL,
        0x202008004008002ULL,
        0x20020004010100ULL,
        0x2048440040820001ULL,
        0x101002200408200ULL,
        0x40802000401080ULL,
        0x4008142004410100ULL,
        0x2060820c0120200ULL,
        0x1001004080100ULL,
        0x20c020080040080ULL,
        0x2935610830022400ULL,
        0x44440041009200ULL,
        0x280001040802101ULL,
        0x2100190040002085ULL,
        0x80c0084100102001ULL,
        0x4024081001000421ULL,
        0x20030a0244872ULL,
        0x12001008414402ULL,
        0x2006104900a0804ULL,
        0x1004081002402ULL};

    const U64 bishop_magic_numbers[64] = {
        0x40040844404084ULL,
        0x2004208a004208ULL,
        0x10190041080202ULL,
        0x108060845042010ULL,
        0x581104180800210ULL,
        0x2112080446200010ULL,
        0x1080820820060210ULL,
        0x3c0808410220200ULL,
        0x4050404440404ULL,
        0x21001420088ULL,
        0x24d0080801082102ULL,
        0x1020a0a020400ULL,
        0x40308200402ULL,
        0x4011002100800ULL,
        0x401484104104005ULL,
        0x801010402020200ULL,
        0x400210c3880100ULL,
        0x404022024108200ULL,
        0x810018200204102ULL,
        0x4002801a02003ULL,
        0x85040820080400ULL,
        0x810102c808880400ULL,
        0xe900410884800ULL,
        0x8002020480840102ULL,
        0x220200865090201ULL,
        0x2010100a02021202ULL,
        0x152048408022401ULL,
        0x20080002081110ULL,
        0x4001001021004000ULL,
        0x800040400a011002ULL,
        0xe4004081011002ULL,
        0x1c004001012080ULL,
        0x8004200962a00220ULL,
        0x8422100208500202ULL,
        0x2000402200300c08ULL,
        0x8646020080080080ULL,
        0x80020a0200100808ULL,
        0x2010004880111000ULL,
        0x623000a080011400ULL,
        0x42008c0340209202ULL,
        0x209188240001000ULL,
        0x400408a884001800ULL,
        0x110400a6080400ULL,
        0x1840060a44020800ULL,
        0x90080104000041ULL,
        0x201011000808101ULL,
        0x1a2208080504f080ULL,
        0x8012020600211212ULL,
        0x500861011240000ULL,
        0x180806108200800ULL,
        0x4000020e01040044ULL,
        0x300000261044000aULL,
        0x802241102020002ULL,
        0x20906061210001ULL,
        0x5a84841004010310ULL,
        0x4010801011c04ULL,
        0xa010109502200ULL,
        0x4a02012000ULL,
        0x500201010098b028ULL,
        0x8040002811040900ULL,
        0x28000010020204ULL,
        0x6000020202d0240ULL,
        0x8918844842082200ULL,
        0x4010011029020020ULL};

    // The U64 values for bitmaps used to isolate specific files
    const U64 not_a = 18374403900871474942ULL;
    const U64 not_ab = 18229723555195321596ULL;
    const U64 not_h = 9187201950435737471ULL;
    const U64 not_gh = 4557430888798830399ULL;
    const U64 rank_2 = 71776119061217280ULL;
    const U64 rank_7 = 65280ULL;

    // Returns a bitboard of pawn attacks depending on side
    U64 mask_pawn_attacks(bool side, int square) {
        // Initializes empty bitboard
        U64 attacks = 0ULL;

        // Pushes a bit to the location of the requested square
        U64 bitboard = 1ULL << square;

        //Adds moves depending on color and whether square is on an edge
        if (side == white) {
            if (bitboard & not_h) {
                attacks |= bitboard >> 7;
            }
            if (bitboard & not_a) {
                attacks |= bitboard >> 9;
            }
        }
        else {
            if (bitboard & not_h) {
                attacks |= bitboard << 9;
            }
            if (bitboard & not_a) {
                attacks |= bitboard << 7;
            }
        }

        return attacks;
    }

    U64 mask_pawn_quiet_moves(bool side, int square) {

        U64 moves = 0ULL;
        U64 bitboard = 1ULL << square;

        if (side == white) {
            if (bitboard & rank_2) {
                moves |= bitboard >> 16;
            }

            moves |= bitboard >> 8;
        }
        else {
            if (bitboard & rank_7) {
                moves |= bitboard << 16;
            }

            moves |= bitboard << 8;
        }

        return moves;
    }

    // Returns a knight attack mask
    U64 mask_knight_moves(int square) {
        U64 attacks = 0ULL;
        U64 bitboard = 1ULL << square;

        //Excludes moves across board
        if ((bitboard >> 6) & not_ab)
            attacks |= bitboard >> 6;
        if ((bitboard >> 10) & not_gh)
            attacks |= bitboard >> 10;
        if ((bitboard >> 15) & not_a)
            attacks |= bitboard >> 15;
        if ((bitboard >> 17) & not_h)
            attacks |= bitboard >> 17;
        if ((bitboard << 6) & not_gh)
            attacks |= bitboard << 6;
        if ((bitboard << 10) & not_ab)
            attacks |= bitboard << 10;
        if ((bitboard << 15) & not_h)
            attacks |= bitboard << 15;
        if ((bitboard << 17) & not_a)
            attacks |= bitboard << 17;

        return attacks;
    }

    // Returns a king attack mask
    U64 mask_king_moves(int square) {
        U64 attacks = 0ULL;
        U64 bitboard = 1ULL << square;

        attacks |= bitboard >> 8;
        attacks |= bitboard << 8;
        if (bitboard & not_a) {
            attacks |= bitboard >> 1;
            attacks |= bitboard >> 9;
            attacks |= bitboard << 7;
        }
        if (bitboard & not_h) {
            attacks |= bitboard << 1;
            attacks |= bitboard << 9;
            attacks |= bitboard >> 7;
        }

        return attacks;
    }

    // Returns a bishop attack mask
    U64 mask_bishop_attacks(int square) {
        U64 attacks = 0ULL;

        int tr, tf;
        tr = square / 8;
        tf = square % 8;

        // Goes in each direction until board edge is found
        for (int r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++)
            attacks |= 1ULL << r * 8 + f;
        for (int r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--)
            attacks |= 1ULL << r * 8 + f;
        for (int r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++)
            attacks |= 1ULL << r * 8 + f;
        for (int r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--)
            attacks |= 1ULL << r * 8 + f;

        return attacks;
    }

    // Returns a rook attack mask
    U64 mask_rook_attacks(int square) {
        U64 attacks = 0ULL;

        int tr, tf;
        tr = square / 8;
        tf = square % 8;

        // Goes in each direction until board edge is found
        for (int r = tr + 1; r < 7; r++)
            attacks |= 1ULL << r * 8 + tf;
        for (int r = tr - 1; r > 0; r--)
            attacks |= 1ULL << r * 8 + tf;
        for (int f = tf + 1; f < 7; f++)
            attacks |= 1ULL << tr * 8 + f;
        for (int f = tf - 1; f > 0; f--)
            attacks |= 1ULL << tr * 8 + f;

        return attacks;
    }

    // Returns bishop moves depending on blocker bitboard
    U64 bishop_moves_on_the_fly(int square, U64 blockers) {
        U64 attacks = 0ULL;

        int tr = square / 8;
        int tf = square % 8;

        // Goes in each direction until blocker or edge is found
        for (int r = tr + 1, f = tf + 1; r < 8 && f < 8; r++, f++) {
            attacks |= 1ULL << r * 8 + f;
            if ((1ULL << r * 8 + f) & blockers)
                break;
        }
        for (int r = tr + 1, f = tf - 1; r < 8 && f >= 0; r++, f--) {
            attacks |= 1ULL << r * 8 + f;
            if ((1ULL << r * 8 + f) & blockers)
                break;
        }
        for (int r = tr - 1, f = tf + 1; r >= 0 && f < 8; r--, f++) {
            attacks |= 1ULL << r * 8 + f;
            if ((1ULL << r * 8 + f) & blockers)
                break;
        }
        for (int r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
            attacks |= 1ULL << r * 8 + f;
            if ((1ULL << r * 8 + f) & blockers)
                break;
        }

        return attacks;
    }

    // Returns rook moves depending on blocker bitboard
    U64 rook_moves_on_the_fly(int square, U64 blockers) {
        U64 attacks = 0ULL;

        int tr = square / 8;
        int tf = square % 8;

        // Goes in each direction until blocker or edge is found
        for (int r = tr + 1; r < 8; r++) {
            attacks |= 1ULL << r * 8 + tf;
            if ((1ULL << r * 8 + tf) & blockers)
                break;
        }
        for (int r = tr - 1; r >= 0; r--) {
            attacks |= 1ULL << r * 8 + tf;
            if ((1ULL << r * 8 + tf) & blockers)
                break;
        }
        for (int f = tf + 1; f < 8; f++) {
            attacks |= 1ULL << tr * 8 + f;
            if ((1ULL << tr * 8 + f) & blockers)
                break;
        }
        for (int f = tf - 1; f >= 0; f--) {
            attacks |= 1ULL << tr * 8 + f;
            if ((1ULL << tr * 8 + f) & blockers)
                break;
        }

        return attacks;
    }

    // Generates the appropriate bitboard from a permutation and attack_mask
    U64 set_occupancy(int permutation, int num_bits, U64 attack_mask) {
        U64 occupancy = 0ULL;
        int square;
        for (int count = 0; count < num_bits; count++) {
            square = util::get_ls1b(attack_mask);
            pop_bit(attack_mask, square);
            if (permutation & (1 << count))
                set_bit(occupancy, square);
        }
        return occupancy;
    }

    // Initializes the different leaper moves
    void init_leaper_moves() {
        for (int square = 0; square < 64; square++) {
            pawn_attacks[white][square] = mask_pawn_attacks(white, square);
            pawn_quiet_moves[white][square] = mask_pawn_quiet_moves(white, square);
            pawn_attacks[black][square] = mask_pawn_attacks(black, square);
            pawn_quiet_moves[black][square] = mask_pawn_quiet_moves(black, square);
            knight_moves[square] = mask_knight_moves(square);
            king_moves[square] = mask_king_moves(square);
        }
    }

    // Initializes the different slider moves
    void init_slider_moves(bool bishop) {
        for (int square = 0; square < 64; square++) {
            bishop_masks[square] = mask_bishop_attacks(square);
            rook_masks[square] = mask_rook_attacks(square);

            U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

            int relevant_bits = util::count_bits(attack_mask);

            int max_occupancy_index = 1 << relevant_bits;

            for (int i = 0; i < max_occupancy_index; i++) {
                if (bishop) {
                    U64 occupancy = set_occupancy(i, relevant_bits, attack_mask);

                    int magic_index = (occupancy * bishop_magic_numbers[square] >> (64 - bishop_relevant_bits[square]));

                    bishop_attacks[square][magic_index] = bishop_moves_on_the_fly(square, occupancy);
                }

                else {
                    U64 occupancy = set_occupancy(i, relevant_bits, attack_mask);

                    int magic_index = (occupancy * rook_magic_numbers[square] >> (64 - rook_relevant_bits[square]));

                    rook_attacks[square][magic_index] = rook_moves_on_the_fly(square, occupancy);
                }
            }
        }
    }

    // Performs complete setup
    void init() {
        init_leaper_moves();
        init_slider_moves(bishop);
        init_slider_moves(rook);
    }

    // Generates a random 64-bit number with fewer 1's
    U64 generate_magic_number_contender() {
        return rng::generate_64_bit() & rng::generate_64_bit() & rng::generate_64_bit();
    }

    // Generates a valid magic number for a square and sliding piece type
    U64 find_magic_number(int square, int relevant_bits, int bishop) {

        // There can be at max 2^12 = 4096 different permutations of occupancies
        U64 occupancies[4096];

        U64 attacks[4096];

        U64 used_attacks[4096];

        U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

        // This is used since the maximum index varies depending on how many squares a given piece can move to
        int max_occupancy_index = 1 << relevant_bits;
        for (int i = 0; i < max_occupancy_index; i++) {
            occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);

            attacks[i] = bishop ? bishop_moves_on_the_fly(square, occupancies[i]) : rook_moves_on_the_fly(square, occupancies[i]);
        }

        for (int random_count = 0; random_count < 100000000; random_count++) {
            U64 magic_number = generate_magic_number_contender();

            if (util::count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) {
                continue;
            }

            memset(used_attacks, 0ULL, sizeof(used_attacks));

            bool failed = false;
            for (int i = 0; i < max_occupancy_index && !failed; i++) {
                int magic_index = (int)((occupancies[i] * magic_number) >> (64 - relevant_bits));

                if (used_attacks[magic_index] == 0ULL) {
                    used_attacks[magic_index] = attacks[i];
                }

                else if (used_attacks[magic_index] != attacks[i]) {
                    failed = true;
                }
            }

            if (!failed) {
                return magic_number;
            }
        }

        cout << "No magic number could be found\n";
        return 0ULL;
    }

    // Used to format and print magic numbers to use for move generation
    void print_magic_numbers() {
        for (int square = 0; square < 64; square++) {
            printf(" 0x%llxUll,\n", find_magic_number(square, rook_relevant_bits[square], rook));
        }

        cout << "\n\n";

        for (int square = 0; square < 64; square++) {
            printf(" 0x%llxUll,\n", find_magic_number(square, bishop_relevant_bits[square], bishop));
        }
    }

    // The following leaper piece attack functions simply return
    // the attacks from the corresponding attack array index
    static inline U64 get_pawn_attacks(int side, int square) {
        return pawn_attacks[side][square];
    }

    static inline U64 get_knight_moves(int square) {
        return knight_moves[square];
    }

    static inline U64 get_king_moves(int square) {
        return king_moves[square];
    }

    // The following slider piece attack functions return
    // the corresponding piece mask, using magic numbers along the way
    // to get relevant index for the piece attack array.
    static inline U64 get_bishop_attacks(int square, U64 occupancy) {
        // Filters out squares the piece cannot move to
        occupancy &= bishop_masks[square];

        // Occupancy becomes magic index for bishop attack retrieval by using corresponding magic number
        occupancy *= bishop_magic_numbers[square];
        occupancy >>= 64 - bishop_relevant_bits[square];

        return bishop_attacks[square][occupancy];
    }

    static inline U64 get_rook_attacks(int square, U64 occupancy) {
        occupancy &= rook_masks[square];
        occupancy *= rook_magic_numbers[square];
        occupancy >>= 64 - rook_relevant_bits[square];
        return rook_attacks[square][occupancy];
    }

    static inline U64 get_queen_attacks(int square, U64 occupancy) {
        // Simply combines possible rook and bishop moves
        return get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy);
    }

    // Used to find out if a given square is attacked
    static inline int is_square_attacked(int square, int side) {

        // The key point is that any piece (with the exception of pawns) can reach the same square it moved from
        // A pawn of opposite color should overlap with one of the white pawns' attacks
        if (side == white && (move_gen::get_pawn_attacks(black, square) & state::bitboards[P]))
            return 1;
        if (side == black && (move_gen::get_pawn_attacks(white, square) & state::bitboards[p]))
            return 1;
        if (move_gen::get_knight_moves(square) & ((side == white) ? state::bitboards[N] : state::bitboards[n]))
            return 1;
        if (move_gen::get_king_moves(square) & ((side == white) ? state::bitboards[K] : state::bitboards[k]))
            return 1;

        // Sliders rely on current occupancy
        if (move_gen::get_bishop_attacks(square, state::occupancies[both]) & ((side == white) ? state::bitboards[B] : state::bitboards[b]))
            return 1;
        if (move_gen::get_rook_attacks(square, state::occupancies[both]) & ((side == white) ? state::bitboards[R] : state::bitboards[r]))
            return 1;
        if (move_gen::get_queen_attacks(square, state::occupancies[both]) & ((side == white) ? state::bitboards[Q] : state::bitboards[q]))
            return 1;

        return 0;
    }
    
    // Used to add a move to a move list
    static inline void add_move(moves *move_list, int move) {
        move_list->array[move_list->size] = move;
        ++move_list->size;
    }

    // Used to generate all possible moves
    static inline void generate_moves(moves *move_list) {
        // Reset move count
        move_list->size = 0;

        // Define source & target squares
        int source_square, target_square;

        // Define current piece's bitboard copy and its attacks
        U64 bitboard, attacks;

        // Generate white pawns & white king castling moves
        if (state::side == white) {
            bitboard = state::bitboards[P];

            while (bitboard) {
                source_square = util::get_ls1b(bitboard);
                target_square = source_square - 8;

                // Generate quiet pawn moves
                if (!get_bit(state::occupancies[both], target_square)) {
                    // Pawn promotion
                    if (source_square >= a7 && source_square <= h7) {
                        add_move(move_list, encode_move(source_square, target_square, P, Q, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, R, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, B, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, N, no_piece, 0, 0, 0));
                    }

                    else {
                        // One square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, P, 0, no_piece, 0, 0, 0));

                        // Two squares ahead pawn move
                        if ((source_square >= a2 && source_square <= h2) && !get_bit(state::occupancies[both], target_square - 8)) {
                            add_move(move_list, encode_move(source_square, target_square - 8, P, 0, no_piece, 1, 0, 0));
                        }
                    }
                }

                // Init pawn attacks bitboard
                attacks = get_pawn_attacks(state::side, source_square) & state::occupancies[black];

                while (attacks) {
                    target_square = util::get_ls1b(attacks);
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white) {
                        start_piece = p;
                        end_piece = k;
                    }
                    else {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++) {
                        if (is_occupied(state::bitboards[piece_type], target_square)) {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // Pawn promotion
                    if (source_square >= a7 && source_square <= h7) {

                        add_move(move_list, encode_move(source_square, target_square, P, Q, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, R, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, B, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, P, N, target_piece, 0, 0, 0));
                    }

                    else {
                        add_move(move_list, encode_move(source_square, target_square, P, 0, target_piece, 0, 0, 0));
                    }

                    // Pop ls1b of the pawn attacks
                    pop_bit(attacks, target_square);
                }

                // Generate en_passant captures
                if (state::en_passant != no_sq) {
                    // Lookup pawn attacks and bitwise AND with en_passant square (bit)
                    U64 en_passant_attacks = get_pawn_attacks(state::side, source_square) & (1ULL << state::en_passant);

                    // Make sure en_passant capture is available
                    if (en_passant_attacks) {
                        // Init en_passant capture target square
                        int target_en_passant = util::get_ls1b(en_passant_attacks);
                        add_move(move_list, encode_move(source_square, target_en_passant, P, 0, p, 0, 1, 0));
                    }
                }

                // Pop ls1b from piece bitboard copy
                pop_bit(bitboard, source_square);
            }

            // Castling moves
            bitboard = state::bitboards[K];

            // King side castling is available
            if (state::castle & wk) {
                // Make sure square between king and king's rook are empty
                if (!get_bit(state::occupancies[both], f1) && !get_bit(state::occupancies[both], g1)) {
                    // Make sure king and the f1 squares are not under attacks
                    if (!move_gen::is_square_attacked(e1, black) && !move_gen::is_square_attacked(f1, black)) {
                        add_move(move_list, encode_move(e1, g1, K, 0, no_piece, 0, 0, 1));
                    }
                }
            }

            // Queen side castling is available
            if (state::castle & wq) {
                // Make sure square between king and queen's rook are empty
                if (!get_bit(state::occupancies[both], d1) && !get_bit(state::occupancies[both], c1) && !get_bit(state::occupancies[both], b1)) {
                    // Make sure king and the d1 squares are not under attacks
                    if (!move_gen::is_square_attacked(e1, black) && !move_gen::is_square_attacked(d1, black)) {
                        add_move(move_list, encode_move(e1, c1, K, 0, no_piece, 0, 0, 1));
                    }
                }
            }
        }

        // Generate black pawns & black king castling moves
        else {
            bitboard = state::bitboards[p];

            while (bitboard) {
                source_square = util::get_ls1b(bitboard);

                target_square = source_square + 8;

                // Generate quiet pawn moves
                if (!get_bit(state::occupancies[both], target_square)) {
                    // Pawn promotion
                    if (source_square >= a2 && source_square <= h2) {
                        add_move(move_list, encode_move(source_square, target_square, p, q, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, r, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, b, no_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, n, no_piece, 0, 0, 0));
                    }

                    else {
                        // One square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, p, 0, no_piece, 0, 0, 0));

                        // Two squares ahead pawn move
                        if ((source_square >= a7 && source_square <= h7) && !get_bit(state::occupancies[both], target_square + 8)) {
                            add_move(move_list, encode_move(source_square, target_square + 8, p, 0, no_piece, 1, 0, 0));
                        }
                    }
                }

                // Init pawn attacks bitboard
                attacks = get_pawn_attacks(state::side, source_square) & state::occupancies[white];

                // Generate pawn captures
                while (attacks) {
                    target_square = util::get_ls1b(attacks);
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white) {
                        start_piece = p;
                        end_piece = k;
                    }
                    else {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++) {
                        if (is_occupied(state::bitboards[piece_type], target_square)) {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }

                    // Pawn promotion
                    if (source_square >= a2 && source_square <= h2) {
                        add_move(move_list, encode_move(source_square, target_square, p, q, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, r, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, b, target_piece, 0, 0, 0));
                        add_move(move_list, encode_move(source_square, target_square, p, n, target_piece, 0, 0, 0));
                    }

                    else {
                        // One square ahead pawn move
                        add_move(move_list, encode_move(source_square, target_square, p, 0, target_piece, 0, 0, 0));
                    }

                    pop_bit(attacks, target_square);
                }

                // Generate en_passant captures
                if (state::en_passant != no_sq) {
                    // Lookup pawn attacks and bitwise AND with en_passant square (bit)
                    U64 en_passant_attacks = get_pawn_attacks(state::side, source_square) & (1ULL << state::en_passant);

                    if (en_passant_attacks) {
                        int target_en_passant = util::get_ls1b(en_passant_attacks);
                        add_move(move_list, encode_move(source_square, target_en_passant, p, 0, P, 0, 1, 0));
                    }
                }

                pop_bit(bitboard, source_square);
            }

            bitboard = state::bitboards[k];

            if (state::castle & bk) {
                // Make sure square between king and king's rook are empty
                if (!get_bit(state::occupancies[both], f8) && !get_bit(state::occupancies[both], g8)) {
                    // Make sure king and the f8 squares are not under attacks
                    if (!move_gen::is_square_attacked(e8, white) && !move_gen::is_square_attacked(f8, white))
                        add_move(move_list, encode_move(e8, g8, k, 0, no_piece, 0, 0, 1));
                }
            }

            // Queen side castling is available
            if (state::castle & bq) {
                // Make sure square between king and queen's rook are empty
                if (!get_bit(state::occupancies[both], d8) && !get_bit(state::occupancies[both], c8) && !get_bit(state::occupancies[both], b8)) {
                    // Make sure king and the d8 squares are not under attacks
                    if (!move_gen::is_square_attacked(e8, white) && !move_gen::is_square_attacked(d8, white))
                        add_move(move_list, encode_move(e8, c8, k, 0, no_piece, 0, 0, 1));
                }
            }
        }

        int piece = (state::side == white ? N : n);
        bitboard = state::bitboards[piece];

        // Loop over source squares of piece bitboard copy
        while (bitboard)
        {
            source_square = util::get_ls1b(bitboard);

            // Init piece attacks in order to get set of target squares
            attacks = get_knight_moves(source_square) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            while (attacks)
            {
                target_square = util::get_ls1b(attacks);

                // Quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square)) {
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));
                }

                else {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white) {
                        start_piece = p;
                        end_piece = k;
                    }
                    else {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++) {
                        if (is_occupied(state::bitboards[piece_type], target_square)) {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }
                    // Capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                pop_bit(attacks, target_square);
            }

            pop_bit(bitboard, source_square);
        }

        // Generate bishop moves
        piece = (state::side == white ? B : b);
        bitboard = state::bitboards[piece];

        while (bitboard) {
            source_square = util::get_ls1b(bitboard);

            attacks = get_bishop_attacks(source_square, state::occupancies[both]) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            while (attacks) {
                target_square = util::get_ls1b(attacks);

                // Quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square)) {
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));
                }

                else {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white) {
                        start_piece = p;
                        end_piece = k;
                    }
                    else {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++) {
                        if (is_occupied(state::bitboards[piece_type], target_square)) {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }

                    // Capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                pop_bit(attacks, target_square);
            }

            pop_bit(bitboard, source_square);
        }

        // Generate rook moves
        piece = (state::side == white ? R : r);
        bitboard = state::bitboards[piece];
        while (bitboard) {
            source_square = util::get_ls1b(bitboard);

            attacks = get_rook_attacks(source_square, state::occupancies[both]) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            while (attacks) {
                target_square = util::get_ls1b(attacks);

                // Quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square)) {
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));
                }

                else {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white) {
                        start_piece = p;
                        end_piece = k;
                    }
                    else {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++) {
                        if (is_occupied(state::bitboards[piece_type], target_square)) {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }

                    // Capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                pop_bit(attacks, target_square);
            }

            pop_bit(bitboard, source_square);
        }

        // Generate queen moves
        piece = (state::side == white ? Q : q);
        bitboard = state::bitboards[piece];
        while (bitboard) {
            source_square = util::get_ls1b(bitboard);

            attacks = get_queen_attacks(source_square, state::occupancies[both]) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            while (attacks) {
                target_square = util::get_ls1b(attacks);

                // Quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square)) {
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));
                }

                else {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white) {
                        start_piece = p;
                        end_piece = k;
                    }
                    else {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++) {
                        if (is_occupied(state::bitboards[piece_type], target_square)) {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }

                    // Capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                pop_bit(attacks, target_square);
            }

            pop_bit(bitboard, source_square);
        }

        // Generate king moves
        piece = (state::side == white ? K : k);
        bitboard = state::bitboards[piece];
        while (bitboard) {
            source_square = util::get_ls1b(bitboard);

            attacks = get_king_moves(source_square) & ((state::side == white) ? ~state::occupancies[white] : ~state::occupancies[black]);

            while (attacks) {
                target_square = util::get_ls1b(attacks);

                // Quiet move
                if (!get_bit(((state::side == white) ? state::occupancies[black] : state::occupancies[white]), target_square)) {
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, no_piece, 0, 0, 0));
                }

                else {
                    int target_piece = no_piece;
                    int start_piece, end_piece;
                    if (state::side == white) {
                        start_piece = p;
                        end_piece = k;
                    }
                    else {
                        start_piece = P;
                        end_piece = K;
                    }

                    for (int piece_type = start_piece; piece_type <= end_piece; piece_type++) {
                        if (is_occupied(state::bitboards[piece_type], target_square)) {
                            target_piece = piece_type;
                            // for some reason, break makes it slower
                            // break;
                        }
                    }

                    // Capture move
                    add_move(move_list, encode_move(source_square, target_square, piece, 0, target_piece, 0, 0, 0));
                }
                pop_bit(attacks, target_square);
            }

            pop_bit(bitboard, source_square);
        }
    }
}

// Macros for copying and reversing the current board state
#define copy_state(move) \
    int move_copy = (move | (state::castle << 24));

#define revert_state() \
    move_exec::undo_move(move_copy)

/*
    The move_exec namespace contains functions and algorithms to make moves on the board
*/
namespace move_exec {
    // Move sorting helper arrays
    int killer_moves[2][246];
    int history_moves[12][246];
    int pv_length[246];
    int pv_table[246][246];

    // The current ply depth of calculation (ply means half-move)
    int ply = 0;

    // Amount of nodes reached (used for time management and debugging)
    std::uint64_t nodes = 0;

    // Constant for null-move pruning
    const int reduced_depth_factor = 2;

    // Timer used for time management
    Timer timer;

    // Variables used for time management
    bool stop_calculating = false;
    bool use_time = false;
    double stop_time = std::numeric_limits<double>::infinity();

    static inline void check_if_time_is_up() {
        if (use_time && timer.get_time_passed_millis() > stop_time) {
            stop_calculating = true;
        }
    }

    // Updates the combined occupancy bitboard
    static inline void merge_occupancies() {
        state::occupancies[both] = (state::occupancies[white] | state::occupancies[black]);
    }

    // Sets the occupancy bitboards for each side
    static inline void populate_occupancies() {
        memset(state::occupancies, 0ULL, OCCUPANCIES_SIZE);

        for (int piece_type = P; piece_type <= K; piece_type++) {
            state::occupancies[white] |= state::bitboards[piece_type];
        }

        for (int piece_type = p; piece_type <= k; piece_type++) {
            state::occupancies[black] |= state::bitboards[piece_type];
        }

        merge_occupancies();
    }

    // Used to evaluate a move and give it a score
    static inline int score_move(int move) {

        // Initialized to P since en passant capture
        // doesn't capture on target square
        int target_piece = P;

        // Score capture move
        if (is_capture(move)) {
            return mvv_lva[get_piece(move)][get_captured_piece_type(move)] + 10000;
        }

        // Score quiet move (non-capture)
        else {

            // If move is a first priority killer move
            if (move == killer_moves[0][ply]) {
                return 9000;
            }

            else if (move == killer_moves[1][ply]) {
                return 8000;
            }
        
            return history_moves[get_piece(move)][get_target(move)];
        }
    }

    // Sorts moves based on their score
    static inline void sort_moves(moves *move_list) {
        int move_scores[move_list->size];

        for (int i = 0; i < move_list->size; i++) {
            move_scores[i] = score_move(move_list->array[i]);
        }

        for (int current_move = 0; current_move < move_list->size; current_move++) {
            for (int next_move = current_move + 1; next_move < move_list->size; next_move++) {
                if (move_scores[next_move] > move_scores[current_move]) {
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

    // Used to undo move on the board
    static inline void undo_move(int move) {
        state::side ^= 1;

        int source = get_source(move);
        int target = get_target(move);
        int piece = get_piece(move);
        int promotion_piece_type = get_promotion_piece_type(move);
        int captured_piece = get_captured_piece_type(move);

        // Move piece back
        pop_bit(state::bitboards[promotion_piece_type ? promotion_piece_type : piece], target);
        pop_bit(state::occupancies[state::side], target);
        set_bit(state::bitboards[piece], source);
        set_bit(state::occupancies[state::side], source);

        // If the move was en passant, put the captured pawn back
        if (is_en_passant(move)) {
            state::en_passant = target;

            if (state::side == white) {
                set_bit(state::bitboards[p], target + 8);
                set_bit(state::occupancies[black], target + 8);
            }
            else {
                set_bit(state::bitboards[P], target - 8);
                set_bit(state::occupancies[white], target - 8);
            }
        }

        // If move was a capture, put the captured piece back
        else if (is_capture(move)) {

            set_bit(state::bitboards[captured_piece], target);
            set_bit(state::occupancies[state::side ^ 1], target);
        }

        // If move was castling, puts back the rook to the corner
        else if (is_castling(move)) {
            switch (target) {
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

        // Sets the castling state
        state::castle = get_castle(move);

        // Update occupancies
        merge_occupancies();
    }

    // Used to make a move on the board
    static inline int make_move(int move) {

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
        if (is_en_passant(move)) {
            if (state::side == white) {
                pop_bit(state::bitboards[p], target + 8);
                pop_bit(state::occupancies[black], target + 8);
            }
            else {
                pop_bit(state::bitboards[P], target - 8);
                pop_bit(state::occupancies[white], target - 8);
            }
        }

        // If move is a capture, remove the attacked piece
        else if (is_capture(move)) {
            pop_bit(state::bitboards[get_captured_piece_type(move)], target);
            pop_bit(state::occupancies[state::side ^ 1], target);
        }

        // Set en passant square if a double pawn push was made
        else if (is_double_pawn_push(move)) {
            state::side == white ? state::en_passant = target + 8 : state::en_passant = target - 8;
        }

        // If move is castling, moves the appropriate rook
        else if (is_castling(move)) {
            switch (target) {
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
        merge_occupancies();

        // Switch sides
        state::side ^= 1;

        // Check that the king is not in check
        if (move_gen::is_square_attacked((state::side == black) ? util::get_ls1b(state::bitboards[K]) : util::get_ls1b(state::bitboards[k]), state::side)) {
            // If it is, revert back and return illegal move
            revert_state();
            return 0;
        }

        // Otherwise, return legal move
        return 1;
    }

    // Evaluates the board state
    static inline int eval() {
        U64 bitboard_copy;
        int square, score;

        for (int piece_type = P; piece_type <= k; piece_type++) {
            bitboard_copy = state::bitboards[piece_type];

            while (bitboard_copy) {
                square = util::get_ls1b(bitboard_copy);
                pop_bit(bitboard_copy, square);
                score += piece_score[piece_type][square];
            }
        }

        return (state::side == white ? score : -score);
    }

    // Performs quiescence search with alpha-beta pruning
    // https://www.chessprogramming.org/Quiescence_Search
    static inline int quiescence(int alpha, int beta) {
        // Check if command should terminate based on time spent calculating
        if (!(nodes % 4000)) {
            check_if_time_is_up();
        }

        ++nodes;

        int evaluation = eval();

        if (evaluation >= beta) {
            return beta;
        }

        if (evaluation > alpha) {
            alpha = evaluation;
        }

        moves move_list[1];
        move_gen::generate_moves(move_list);
        sort_moves(move_list);

        for (int i = 0; i < move_list->size; i++) {
            int current_move = move_list->array[i];

            copy_state(current_move);

            ++ply;

            // If move is illegal or not a capture
            if (!is_capture(move_list->array[i]) || !make_move(move_list->array[i])) {
                --ply;
                continue;
            }

            // Recursively determines if capture chain is beneficial
            int score = -quiescence(-beta, -alpha);
            --ply;

            revert_state();

            if (stop_calculating) {
                return 0;
            }

            if (score >= beta) {
                return beta;
            }

            if (score > alpha) {
                alpha = score;
            }
        }

        return alpha;
    }

    // Implementation of the minimax algorithm with negamax
    // https://www.chessprogramming.org/Negamax
    static inline int negamax(int alpha, int beta, int depth) {
        pv_length[ply] = ply;

        if (!depth) {
            return quiescence(alpha, beta);
        }

        ++nodes;

        // Note whether king is currently in check
        int in_check = move_gen::is_square_attacked(
            (state::side == white ? util::get_ls1b(state::bitboards[K]) : util::get_ls1b(state::bitboards[k])),
            state::side ^ 1
        );

        if (in_check) {
            ++depth;
        }

        // Null-move pruning
        // https://web.archive.org/web/20071031095933/http://www.brucemo.com/compchess/programming/nullmove.htm
        if (depth >= 3 && !in_check && ply) {
            int side_copy = state::side;
            int en_passant_copy = state::en_passant;

            // Imitates board as if it is opponent to move
            state::side ^= 1;
            state::en_passant = no_sq;

            int score = -negamax(-beta, -beta + 1, depth - 1 - reduced_depth_factor);

            state::side = side_copy;
            state::en_passant = en_passant_copy;

            if (stop_calculating) {
                return 0;
            }

            if (score >= beta) {
                return beta;
            }
        }

        // Keep track of the amount of legal moves
        int legal_moves = 0;

        // Move list init and find all moves
        moves move_list[1];
        move_gen::generate_moves(move_list);
        sort_moves(move_list);

        for (int i = 0; i < move_list->size; i++) {
            int current_move = move_list->array[i];

            copy_state(current_move);

            ++ply;

            // If move is illegal
            if (!make_move(current_move)) {
                --ply;
                continue;
            }

            ++legal_moves;

            // Update score recursively with the negamax property
            int score = -negamax(-beta, -alpha, depth - 1);
            --ply;

            revert_state();

            if (stop_calculating) {
                return 0;
            }

            // If a new, better move has been found
            if (score >= beta) {
                if (!is_capture(current_move)) {
                    // Stores killer move for current ply
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = current_move;
                }

                return beta;
            }

            if (score > alpha) {
                // Stores history move depending on piece type and target square.
                // The idea is to improve score slightly every time a specific
                // move is seen to improve the position.
                // Higher depths are rewarded more, since deeper calculation is
                // generally more correct.
                if (!is_capture(current_move)) {
                    history_moves[get_piece(current_move)][get_target(current_move)] += depth;
                }

                alpha = score;

                pv_table[ply][ply] = current_move;

                for (int next_ply = ply + 1; next_ply < pv_length[ply + 1]; next_ply++) {
                    pv_table[ply][next_ply] = pv_table[ply + 1][next_ply];
                }

                pv_length[ply] = pv_length[ply + 1];
            }
        }

        // If there are no legal moves
        if (!legal_moves) {
            // King is in check (checkmate)
            if (in_check) {
                // "+ ply" prioritizes shorter checkmates
                return -49000 + ply;
            }

            // King is not in check (stalemate)
            return 0;
        }

        return alpha;
    }

    // Function that binds everything together and looks for the best move, up to some depth
    // Takes into account time, killer moves, history moves, and the principle variation, for efficiency 
    void search_position(int depth) {
        stop_calculating = false;

        // Resets helper arrays
        memset(killer_moves, 0, sizeof(killer_moves));
        memset(history_moves, 0, sizeof(history_moves));
        memset(pv_length, 0, sizeof(pv_length));
        memset(pv_table, 0, sizeof(pv_table));

        int alpha = -50000;
        int beta = 50000;
        int candidate_pv_table_copy[246][246];
        
        U64 bitboards_copy[12], occupancies_copy[3];                              
        int side_copy, en_passant_copy, castle_copy;                              
        memcpy(bitboards_copy, state::bitboards, BITBOARDS_SIZE);       
        memcpy(occupancies_copy, state::occupancies, OCCUPANCIES_SIZE); 
        side_copy = state::side, en_passant_copy = state::en_passant, castle_copy = state::castle;

        for (int current_depth = 1; current_depth <= depth; current_depth++) {
            if (stop_calculating) break;

            memcpy(&candidate_pv_table_copy, &pv_table, sizeof(pv_table));

            nodes = 0;

            int score = move_exec::negamax(alpha, beta, current_depth);

            cout << endl;
            if (!stop_calculating) {
                cout << "Found best move at depth " << current_depth << " looking through " << nodes << " nodes" << endl;
                
                for (int i = 0; i < pv_length[0]; i++) {
                    move_exec::make_move(pv_table[0][i]);
                }
                int current_eval = quiescence(alpha, beta);
                cout << "Evaluation: " << format::eval(current_eval) << endl;

                memcpy(state::bitboards, bitboards_copy, BITBOARDS_SIZE);       
                memcpy(state::occupancies, occupancies_copy, OCCUPANCIES_SIZE); 
                state::side = side_copy, state::en_passant = en_passant_copy, state::castle = castle_copy;
            }
            else {
                cout << "Interrupted by time at depth " << current_depth << " looking through " << nodes << " nodes" << endl;
            }
            cout << "Total time passed: " << timer.get_time_passed_millis() << " milliseconds." << endl;
            for (int i = 0; i < pv_length[0]; i++) {
                cout << format::move(pv_table[0][i]) << " ";
            }
            if (pv_length[0]) cout << endl;
        }

        if (!(stop_calculating)) {
            memcpy(&candidate_pv_table_copy, &pv_table, sizeof(pv_table));
        }

        cout << "\nbestmove " << format::move(candidate_pv_table_copy[0][0]) << "\n\n";
    }
}

/*
    The parse namespace so far only used to parse fen strings and set the board state.
*/
namespace parse {
    void fen(std::string fen) {
        memset(state::bitboards, 0ULL, BITBOARDS_SIZE);

        state::side = 0;
        state::en_passant = no_sq;
        state::castle = 0;

        int i = 0;
        int square;

        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                square = rank * 8 + file;

                if ((fen[i] >= 'A' && fen[i] <= 'Z') || (fen[i] >= 'a' && fen[i] <= 'z')) {
                    set_bit(state::bitboards[char_pieces[fen[i]]], square);
                }

                else if (fen[i] >= '0' && fen[i] <= '9') {
                    int offset = fen[i] - '0';
                    int piece = -1;

                    for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                        if (is_occupied(state::bitboards[bb_piece], square))
                            piece = bb_piece;
                        break;
                    }

                    if (piece == -1) {
                        file--;
                    }

                    file += offset;
                }

                else {
                    file--;
                }

                i++;
            }
        }

        i++;
        state::side = (fen[i] == 'w' ? white : black);

        i += 2;
        while (fen[i] != ' ') {
            switch (fen[i]) {
                case 'K':
                    state::castle |= wk;
                    break;
                case 'Q':
                    state::castle |= wq;
                    break;
                case 'k':
                    state::castle |= bk;
                    break;
                case 'q':
                    state::castle |= bq;
                    break;
            }
            i++;
        }

        i++;
        if (fen[i] != '-') {
            int file = fen[i] - 'a';
            i++;
            int rank = 8 - (fen[i] - '0');

            state::en_passant = rank * 8 + file;
        }
        else {
            state::en_passant = no_sq;
        }

        move_exec::populate_occupancies();
    }
}

/*
    The print namespace contains functions to print various chess elements.
*/
namespace print {
    void move(int move) {
        cout << format::move(move);
    }

    void bitboard(U64 bitboard) {
        int square;
        cout << "\n";

        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                square = rank * 8 + file;

                if (!file) {
                    cout << "  " << 8 - rank << " ";
                }

                cout << " " << get_bit(bitboard, square);
            }

            cout << "\n";
        }

        cout << "\n     a b c d e f g h\n\n";

        cout << "     bitboard: " << bitboard << "\n";
    }

    void all_moves(moves *move_list) {
        if (!move_list->size) {
            cout << "\n     No moves in move list!";
            return;
        }

        cout << "\n     move    piece   capture   double    en passant    castling    score\n\n";

        for (int move_count = 0; move_count < move_list->size; move_count++) {
            int move = move_list->array[move_count];

            cout << "     " << index_to_square[get_source(move)] <<
            index_to_square[get_target(move)] <<
            char(promoted_pieces[get_promotion_piece_type(move)]) <<
            "   " << ascii_pieces[get_piece(move)] << 
            "       " << (is_capture(move) ? 1 : 0) <<
            "         " << (is_double_pawn_push(move) ? 1 : 0) <<
            "         " << (is_en_passant(move) ? 1 : 0) << 
            "             " << (is_castling(move) ? 1 : 0) << 
            "           " << (move_exec::score_move(move)) << endl;
        }
        
        cout << "\n     Total number of moves: " << move_list->size << "\n\n";
    }

    void attacked_squares(int side) {
        U64 result = 0ULL;
        for (int square = 0; square < 64; square++) {
            if (move_gen::is_square_attacked(square, side)) set_bit(result, square);
        }
        
        bitboard(result);
    }

    void game() {
        int square;
        cout << "\n";

        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                square = rank * 8 + file;

                if (!file) {
                    cout << "  " << 8 - rank << " ";
                }

                int piece = -1;
                for(int i = 0; i < 12; i++) {
                    if (is_occupied(state::bitboards[i], square)) {
                        piece = i;
                        break;
                    }
                }
                cout << " " << (piece == -1 ? "." : ascii_pieces[piece]);
            }
            cout << "\n";
        }

        cout << "\n     a b c d e f g h\n\n";
        cout << "     Side:        " << (state::side == white ? "white" : "black") << endl;
        cout << "     en_passant:  " << ((state::en_passant != no_sq) ? index_to_square[state::en_passant] : "no") << endl;
        cout << "     Castling:    " << ((state::castle & wk) ? 'K' : '-') <<
                                        ((state::castle & wq) ? 'Q' : '-') <<
                                        ((state::castle & bk) ? 'k' : '-') <<
                                        ((state::castle & bq) ? 'q' : '-') <<
                                        endl;
    }
}

/*
    The perft namespace is used for performance testing.
*/
namespace perft {
    // Amount of reached nodes
    std::uint64_t nodes = 0;

    // Recursive function to test how many possible positions exist
    static inline void driver(int depth) {
        if (!depth) {
            nodes++;
            return;
        }

        moves move_list[1];
        move_gen::generate_moves(move_list);

        for (int move_count = 0; move_count < move_list->size; move_count++) {
            int current_move = move_list->array[move_count];

            // Preserve board state
            copy_state(current_move);
            
            // Makes move and skips if illegal
            if (!move_exec::make_move(current_move)) continue;

            // Recursively calls itself with current position
            driver(depth-1);

            // Retrieves the previous position
            revert_state();
        }
    }

    // Essentially an outer layer of the driver function to display test information.
    void test(int depth) {
        nodes = 0;

        cout << "\n     Performance test\n\n";
        
        moves move_list[1];
        move_gen::generate_moves(move_list);
        
        Timer timer;

        for (int move_count = 0; move_count < move_list->size; move_count++) {
            int current_move = move_list->array[move_count];

            copy_state(current_move);
            
            if (!move_exec::make_move(current_move)) continue;
            
            long cumulative_nodes = nodes;
            
            driver(depth - 1);
            
            long old_nodes = nodes - cumulative_nodes;
            
            revert_state();
            
            cout << "     move: " << format::move(current_move) << "  nodes: " << old_nodes << endl;
        }
        
        cout << "\n    Depth: " << depth;
        cout << "\n    Nodes: " << nodes;
        cout << "\n     Time: " << timer.get_time_passed_millis() << " milliseconds" << endl;
    }
}

/*
    The uci namespace contains functions that implement the universal chess interface.
*/
namespace uci {
    int parse_move(std::string move_string) {
        int source_square = move_string[0] - 'a' + (8 - (move_string[1] - '0')) * 8;
        int target_square = move_string[2] - 'a' + (8 - (move_string[3] - '0')) * 8;

        moves move_list[1];

        move_gen::generate_moves(move_list);

        for (int move_count = 0; move_count < move_list->size; move_count++) {
            int current_move = move_list->array[move_count];
            if (source_square == get_source(current_move) && target_square == get_target(current_move)) {
                int promotion_piece_type = get_promotion_piece_type(current_move) % 6;
                if (!promotion_piece_type) {
                    return current_move;
                }

                switch (move_string[4]) {
                    case 'q':
                        if (promotion_piece_type == Q)
                            return current_move;
                        else
                            return 0;
                        break;

                    case 'r':
                        if (promotion_piece_type == R)
                            return current_move;
                        else
                            return 0;
                        break;

                    case 'b':
                        if (promotion_piece_type == B)
                            return current_move;
                        else
                            return 0;
                        break;

                    case 'n':
                        if (promotion_piece_type == N)
                            return current_move;
                        else
                            return 0;
                        break;

                    default:
                        return 0;
                        break;
                }
            }
        }

        return 0;
    }
    
    void parse_moves(std::string input) {
        // Creates a stringstream from the input string
        std::stringstream ss(input);

        // Uses a vector to store the substrings
        std::vector<std::string> substrings;

        std::string substring;

        // Extracts substrings separated by space and stores them in the vector
        while (ss >> substring) {
            substrings.push_back(substring);
        }

        for (const std::string &str : substrings) {
            if (parse_move(str))
                move_exec::make_move(parse_move(str));
        }
    }

    void print_engine_info() {
        cout << "id name JuulesPlusPlus" << endl;
        cout << "id author Juules32" << endl;
        cout << "uciok" << endl;
    }

    void parse_position(std::string input) {
        size_t position_i = input.find("position");

        if (position_i != std::string::npos) {
            size_t startpos_i = input.find("startpos");
            size_t trickypos_i = input.find("trickypos");
            size_t killerpos_i = input.find("killerpos");
            size_t cmkpos_i = input.find("cmkpos");
            size_t rookpos_i = input.find("rookpos");
            size_t promotionpos_i = input.find("promotionpos");
            size_t checkmatepos_i = input.find("checkmatepos");
            size_t emptypos_i = input.find("emptypos");
            size_t fen_i = input.find("fen");
            size_t moves_i = input.find("moves");

            if (startpos_i != std::string::npos) {
                parse::fen(start_position);
            }
            else if (trickypos_i != std::string::npos) {
                parse::fen(tricky_position);
            }
            else if (killerpos_i != std::string::npos) {
                parse::fen(killer_position);
            }
            else if (cmkpos_i != std::string::npos) {
                parse::fen(cmk_position);
            }
            else if (rookpos_i != std::string::npos) {
                parse::fen(rook_position);
            }
            else if (promotionpos_i != std::string::npos) {
                parse::fen(promotion_position);
            }
            else if (checkmatepos_i != std::string::npos) {
                parse::fen(checkmate_position);
            }
            else if (emptypos_i != std::string::npos) {
                parse::fen(empty_position);
            }

            // Parse and load fen is specified
            else if (fen_i != std::string::npos) {
                parse::fen(input.substr(fen_i + 4));
            }

            // Make moves if specified
            if (moves_i != std::string::npos) {
                uci::parse_moves(input.substr(moves_i + 6));
            }

            print::game();
        }
    }

    void parse_go(std::string input) {
        size_t go_i = input.find("go");

        if (go_i != std::string::npos) {
            size_t depth_i = input.find("depth");
            size_t perft_i = input.find("perft");
            size_t eval_i = input.find("eval");
            size_t wtime_i = input.find("wtime");
            size_t btime_i = input.find("btime");
            size_t winc_i = input.find("winc");
            size_t binc_i = input.find("binc");
            int depth = 6;
            int inc = -1;
            int time = -1;
            int moves_to_go = 30;
            move_exec::use_time = false;
            move_exec::stop_time = std::numeric_limits<double>::infinity();

            if (depth_i != std::string::npos) {
                depth = stoi(input.substr(depth_i + 6));
            }
            else if (perft_i != std::string::npos) {
                perft::test(stoi(input.substr(perft_i + 6)));
                return;
            }
            else if (eval_i != std::string::npos) {
                cout << move_exec::eval() << endl;
                return;
            }

            if (wtime_i != std::string::npos && state::side == white) {
                time = stoi(input.substr(wtime_i + 6));
            }
            if (btime_i != std::string::npos && state::side == black) {
                time = stoi(input.substr(btime_i + 6));
            }
            if (winc_i != std::string::npos && state::side == white) {
                inc = stoi(input.substr(winc_i + 5));
            }
            if (binc_i != std::string::npos && state::side == black) {
                inc = stoi(input.substr(binc_i + 5));
            }

            move_exec::timer.reset();

            if (time != -1) {
                move_exec::use_time = true;

                // - 100 is a small offset to counteract the
                // inevitable delay after stop_time is set to true
                move_exec::stop_time = time / moves_to_go - 100 + inc;

                move_exec::search_position(64);
            }

            else {
                move_exec::search_position(depth);
            }
        }
    }

    // Function that keeps the program running to take commands
    void loop() {
        std::string input;
        while (true) {
            getline(cin, input);
            
            if (input == "quit" || input == "exit") {
                break;
            }

            else if (input == "uci") {
                print_engine_info();
            }

            else if (input == "isready") {
                cout << "readyok" << endl;
            }

            else if (input == "ucinewgame") {
                parse_position("position startpos");
            }

            parse_position(input);
            parse_go(input);
        }
    }

    void init() {
        print_engine_info();
        loop();
    }
}

int main() {
    move_gen::init();

    int debugging = 0;

    if (debugging) {
        // Various code can be tested here for debugging
    }

    else {
        uci::init();
    }
    
    return 0;
}
