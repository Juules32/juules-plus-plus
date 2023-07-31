#include "utils.h"
#include "board.h"

void print::move(int move) {
    cout << "\n     "
        << index_to_square[get_source(move)]
        << index_to_square[get_target(move)]
        << char(promoted_pieces[get_promotion_piece(move)])
        << "\n";
}

void print::all_moves(moves *move_list) {

    if(!move_list->size) {
        cout << "\n    No moves in move list!";
        return;
    }

    cout << "\n    move    piece   capture   double    en passant    castling\n\n";

    for (int move_count = 0; move_count < move_list->size; move_count++)
    {
        int move = move_list->array[move_count];
        printf("    %s%s%c   %c       %d         %d         %d             %d\n", 
        index_to_square[get_source(move)],
        index_to_square[get_target(move)],
        promoted_pieces[get_promotion_piece(move)],
        ascii_pieces[get_piece(move)],
        is_capture(move) ? 1 : 0,
        is_double_pawn_push(move) ? 1 : 0,
        is_en_passant(move) ? 1 : 0,
        is_castling(move) ? 1 : 0);
    }
    
    cout << "\n    Total number of moves: " << move_list->size << "\n\n";
}

void print::attacked_squares(int side) {
    U64 result = 0ULL;
    for (int square = 0; square < 64; square++)
    {
        if(board::is_square_attacked(square, side)) set_bit(result, square);
    }
    
    print::bitboard(result);
}


void print::bitboard(U64 bitboard)
{
    int square;
    cout << "\n";

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            square = rank * 8 + file;

            if (!file)
                cout << "  " << 8 - rank << " ";

            printf(" %d", get_bit(bitboard, square));
        }

        cout << "\n";
    }

    cout << "\n     a b c d e f g h\n\n";

    cout << "     bitboard: " << bitboard << "\n";
}

void print::game() {
    int square;
    cout << "\n";

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            square = rank * 8 + file;

            if (!file)
                cout << "  " << 8 - rank << " ";

            int piece = -1;
            for(int i = 0; i < 12; i++) {
                if(is_occupied(board::bitboards[i], square)) {
                    piece = i;
                    break;
                }
            }

            printf(" %c", piece == -1 ? '.' : ascii_pieces[piece]);

        }

        cout << "\n";
    }

    cout << "\n     a b c d e f g h\n\n";
    printf("     Side:     %s\n", board::side == white ? "white" : "black");
    printf("     en_passant:  %s\n", (board::en_passant != no_sq) ? index_to_square[board::en_passant] : "no");
    printf("     Castling:  %c%c%c%c\n\n", (board::castle & wk) ? 'K' : '-',
                                           (board::castle & wq) ? 'Q' : '-',
                                           (board::castle & bk) ? 'k' : '-',
                                           (board::castle & bq) ? 'q' : '-');
}