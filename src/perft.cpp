#include "perft.h"
#include "utils.h"

std::uint64_t perft::nodes = 0;

void perft::test(int depth)
{
    nodes = 0;

    cout << "\n     Performance test\n\n";
    
    // Create move list instance
    moves move_list[1];
    
    // Generate moves
    board::generate_moves(move_list);
    
    // Init start time
    Timer timer;

    // Loop over generated moves
    for (int move_count = 0; move_count < move_list->size; move_count++)
    {   
        int current_move = move_list->array[move_count];
        

        // Preserve board state
        copy_state(current_move);
        
        // make move
        if (!board::make_move(current_move))
            // skip to the next move
            continue;
        
        // cummulative nodes
        long cummulative_nodes = nodes;
        
        // call perft driver recursively
        perft::driver(depth - 1);
        
        // old nodes
        long old_nodes = nodes - cummulative_nodes;
        
        // take back
        revert_state();

        
        cout << "     move: " << format::move(current_move) << "  nodes: " << old_nodes << endl;
    }
    
    // print results
    cout << "\n    Depth: " << depth;
    cout << "\n    Nodes: " << nodes;
    cout << "\n     Time: " << timer.get_time_passed_millis() << " milliseconds" << endl;
}
