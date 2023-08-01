#include "shorthands.h"
#include "rng.h"
#include "movegen.h"
#include "perft.h"
#include "uci.h"
#include "utils.h"


int main()
{
    bool debugging = true;

    if (debugging)
    {
        board::parse_fen(rook_position);
        cout << board::eval();
        print::game();
    }

    else
    {

        movegen::init();
        uci::init();
    }
    return 0;
}
