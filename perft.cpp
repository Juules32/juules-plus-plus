#include "perft.h"
#include <chrono>
#include <string.h>
#include <iostream>

long perft::nodes = 0;

void perft::test(int depth)
{

    printf("\n     Performance test\n\n");
    
    // Create move list instance
    moves move_list[1];
    
    // Generate moves
    board::generate_moves(move_list);
    
    // Init start time
    auto startTime = chrono::high_resolution_clock::now();
    
    // Loop over generated moves
    for (int move_count = 0; move_count < move_list->size; move_count++)
    {   
        // Preserve board state
        copy_board();
        
        // make move
        if (!board::make_move(move_list->array[move_count]))
            // skip to the next move
            continue;
        
        // cummulative nodes
        long cummulative_nodes = nodes;
        
        // call perft driver recursively
        perft::driver(depth - 1);
        
        // old nodes
        long old_nodes = nodes - cummulative_nodes;
        
        // take back
        revert_board();
        
        // print move
        printf("     move: %s%s%c  nodes: %ld\n", index_to_square[get_source(move_list->array[move_count])],
                                                 index_to_square[get_target(move_list->array[move_count])],
                                                 get_promotion_piece(move_list->array[move_count]) ? promoted_pieces[get_promotion_piece(move_list->array[move_count])] : ' ',
                                                 old_nodes);
    }
    
    // print results
    printf("\n    Depth: %d\n", depth);
    printf("    Nodes: %ld\n", nodes);
    auto endTime = chrono::high_resolution_clock::now();
    cout << "     Time: " << chrono::duration_cast<chrono::milliseconds>(endTime- startTime).count() << endl;
}

