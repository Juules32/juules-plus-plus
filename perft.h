#pragma once
#include "board.h"
#include "move_generation.h"


namespace perft {
    // Amount of reached nodes
    extern long nodes;

    // Recursive function to test how many possible positions exist
    static inline void driver(int depth) {

        if(!depth) {
            nodes++;
            return;
        }

        //generates moves 
        moves move_list[1];
        generate_moves(move_list);

        for (int move_count = 0; move_count < move_list->size; move_count++)
        {
            //copies board for later retrieval
            copy_board();

            //Makes move and skips if illegal
            if(!make_move(move_list->array[move_count], false)) continue;

            //recursively calls itself with current position
            driver(depth-1);

            //retrieves the previous position
            take_back();
        }
    }

    void test(int depth);
}
