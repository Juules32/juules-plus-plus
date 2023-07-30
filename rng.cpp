#include "rng.h"

unsigned int rng::random_state = 1804289383;


// Generates a random 32-bit number from the current state
unsigned int rng::generate_32_bit()
{
    unsigned int number = rng::random_state;

    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // Updates state so new numbers can be found
    random_state = number;

    return number;
}

// Generates a random 64-bit number
unsigned int rng::generate_64_bit()
{

    // Generates four different 32-bit numbers where the first 16 bits are 0
    U64 n1 = (U64)(rng::generate_32_bit()) & 0xFFFF;
    U64 n2 = (U64)(rng::generate_32_bit()) & 0xFFFF;
    U64 n3 = (U64)(rng::generate_32_bit()) & 0xFFFF;
    U64 n4 = (U64)(rng::generate_32_bit()) & 0xFFFF;

    // Slices them all together
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// Generates a random 64-bit number with fewer 1's
U64 rng::generate_magic_number_contender()
{
    return rng::generate_64_bit() & rng::generate_64_bit() & rng::generate_64_bit();
}