#include "shorthands.h"

// Shorthand for getting the index of a square from its name
const char *index_to_square[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

// ASCII pieces
const char ascii_pieces[] = "PNBRQKpnbrqk";

// Unicode pieces (can't be printed to windows console)
const char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

map<char, int> char_pieces = {
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

map<char, int> promoted_pieces = {
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

// FEN dedug positions
string empty_board = "8/8/8/8/8/8/8/8 w - - ";
string start_position = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
string pawns_position = "8/pppppppp/8/8/8/8/PPPPPPPP/8 w KQkq - 0 1 ";
string tricky_position = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
string killer_position = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
string cmk_position = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";
string rook_position = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
