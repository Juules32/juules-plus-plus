
#include <iostream>
#include <string.h>
using namespace std;

#include "shorthands.h"
#include "rng.h"
#include "movegen.h"
#include "perft.h"
#include "uci.h"
#include "utils.h"

int main()
{

    movegen::init();

    board::parse_fen(tricky_position);

    perft::test(5);

    
    getchar(); 
    
    return 0;
}
