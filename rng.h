#pragma once
#define U64 unsigned long long

namespace rng
{
    U64 generate_magic_number_contender();
    unsigned int generate_32_bit();
    unsigned int generate_64_bit();
    
    extern unsigned int random_state;
};
