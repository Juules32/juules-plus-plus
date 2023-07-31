#include "shorthands.h"
#include "rng.h"
#include "movegen.h"
#include "perft.h"
#include "uci.h"
#include "utils.h"

int main()
{
    cout << "\nInitializing ChessPlusPlus engine...\n\n";

    movegen::init();
    uci::init();
    

    
    return 0;
}
