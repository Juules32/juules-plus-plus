
#include <iostream>
#include <string.h>
using namespace std;

#include "shorthands.h"
#include "rng.h"
#include "board.h"
#include "move_generation.h"
#include "perft.h"
#include "uci.h"

int main()
{

    init_moves();

    board::parse_fen(start_position);

    perft::test(6);

    getchar(); 
    
    return 0;
}
