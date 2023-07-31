#pragma once
#include "board.h"

namespace print {
    void move(int move);
    void all_moves(moves *move_list);
    void bitboard(U64 bitboard);
    void game();
    void attacked_squares(int side);
}