#pragma once
#define U64 unsigned long long

/*
    The rng namespace is used for tasks relating to randomness
*/

namespace rng
{
    // Current state, initialized to a large integer
    extern unsigned int random_state;

    // Generates a magic number for slider piece moves
    U64 generate_magic_number_contender();
    
    unsigned int generate_32_bit();
    unsigned int generate_64_bit();
    
};
