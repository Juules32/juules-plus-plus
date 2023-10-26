#include "shorthands.h"
#include "movegen.h"
#include "perft.h"
#include "uci.h"
#include "board.h"

int main()
{
    movegen::init();

    int debugging = 0;

    if (debugging)
    {
        parse::fen(start_position);


        cout << state::castle << endl;
        copy_state(0);
        int move_copy_copy = move_copy;
        // array to store binary number
        int binaryNum[32];
    
        // counter for binary array
        int i = 0;
        while (move_copy_copy > 0) {
    
            // storing remainder in binary array
            binaryNum[i] = move_copy_copy % 2;
            move_copy_copy = move_copy_copy / 2;
            i++;
        }
    
        // printing binary array in reverse order
        for (int j = i - 1; j >= 0; j--) {
            cout << binaryNum[j];

        }
        cout << "\n";
        revert_state();

    }

    else
    {
        uci::init();
    }
    
    return 0;
}
