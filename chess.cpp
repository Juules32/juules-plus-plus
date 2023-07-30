
#include <iostream>
#include <string.h>
using namespace std;

#include "shorthands.h"
#include "rng.h"
#include "bitboards.h"
#include "move_generation.h"
#include "perft.h"
#include "uci.h"

int main()
{

    init_moves();
    parse_fen(tricky_position);

    perft_test(5);    
    
    return 0;
}
