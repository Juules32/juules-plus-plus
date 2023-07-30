#include "uci.h"
#include "move_generation.h"

namespace uci {
    int parse_move(char move_string[])
    {

        int source_square = move_string[0] - 'a' + (8 - (move_string[1] - '0')) * 8;
        int target_square = move_string[2] - 'a' + (8 - (move_string[3] - '0')) * 8;

        moves move_list[1];

        generate_moves(move_list);

        print_moves(move_list);

        for (int move_count = 0; move_count < move_list->size; move_count++)
        {
            int current_move = move_list->array[move_count];
            if (source_square == get_source(current_move) && target_square == get_target(current_move))
            {
                int promotion_piece = get_promotion_piece(current_move) % 6;
                if (!promotion_piece)
                    return current_move;

                switch (move_string[4])
                {
                case 'q':
                    if (promotion_piece == Q)
                        return current_move;
                    else
                        return 0;
                    break;

                case 'r':
                    if (promotion_piece == R)
                        return current_move;
                    else
                        return 0;
                    break;

                case 'b':
                    if (promotion_piece == B)
                        return current_move;
                    else
                        return 0;
                    break;

                case 'n':
                    if (promotion_piece == N)
                        return current_move;
                    else
                        return 0;
                    break;

                default:
                    return 0;
                    break;
                }
            }
        }

        return 0;
    }
}