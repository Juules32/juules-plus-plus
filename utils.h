#pragma once
#include "shorthands.h"

namespace parse {
    void fen(std::string input);
}

namespace format {
    std::string eval(int eval);
    std::string move(int move);
}

/*
    The print namespace contains useful functions
    for printing various chess elements
*/
namespace print {
    void move(int move);
    void all_moves(moves *move_list);
    void bitboard(U64 bitboard);
    void game();
    void attacked_squares(int side);
}

/*
    The rng namespace is used for tasks relating to randomness
*/
namespace rng
{
    // Current state, initialized to a large integer
    extern unsigned int random_state;

    // Generates a magic number for slider piece moves
    U64 generate_magic_number_contender();
    
    // Generates a random 32-bit number
    unsigned int generate_32_bit();

    // Generates a random 64-bit number
    unsigned int generate_64_bit();
    
}

class Timer {
public:
    Timer();
    void reset();
    double get_time_passed_millis();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
};

extern Timer timer;

extern bool stop_calculating;
extern bool use_time;
extern double stop_time;

static inline void check_if_time_is_up() {
    if(use_time && timer.get_time_passed_millis() > stop_time) {
        stop_calculating = true;
    }
}