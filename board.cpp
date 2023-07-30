#include <iostream>
#include "board.h"


U64 board::bitboards[12];

U64 board::occupancies[3];

// side to move
int board::side = -1;

// en_passant square
int board::en_passant = no_sq; 

// castling rights
int board::castle = 0;



// Prints a bitboard
void board::print_bitboard(U64 bitboard)
{
    int square;
    cout << "\n";

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // current square is set
            square = rank * 8 + file;

            // print ranks
            if (!file)
                cout << "  " << 8 - rank << " ";

            // print bit at the current square
            printf(" %d", get_bit(bitboard, square));
        }

        cout << "\n";
    }

    // print files
    cout << "\n     a b c d e f g h\n\n";

    // print bitboard as decimal number
    cout << "     bitboard: " << bitboard << "\n";
}

void board::print_game() {
    int square;
    cout << "\n";

    // loop over board ranks
    for (int rank = 0; rank < 8; rank++)
    {
        // loop over board files
        for (int file = 0; file < 8; file++)
        {
            // current square is set
            square = rank * 8 + file;

            // print ranks
            if (!file)
                cout << "  " << 8 - rank << " ";

            int piece = -1;
            for(int i = 0; i < 12; i++) {
                if(is_occupied(bitboards[i], square)) {
                    piece = i;
                    break;
                }
            }

            printf(" %c", piece == -1 ? '.' : ascii_pieces[piece]);

        }

        cout << "\n";
    }

    // print files
    cout << "\n     a b c d e f g h\n\n";

    // print side to move
    printf("     Side:     %s\n", side == white ? "white" : "black");
    
    // print en_passant square
    printf("     en_passant:  %s\n", (en_passant != no_sq) ? index_to_square[en_passant] : "no");
    
    // print castling rights
    printf("     Castling:  %c%c%c%c\n\n", (castle & wk) ? 'K' : '-',
                                           (castle & wq) ? 'Q' : '-',
                                           (castle & bk) ? 'k' : '-',
                                           (castle & bq) ? 'q' : '-');
}

void board::parse_fen(char fen[]) {
    memset(bitboards, 0ULL, sizeof(bitboards));

    side = 0;
    en_passant = no_sq;
    castle = 0;


    int i = 0;
    int square;

    for(int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++)
        {
            square = rank*8 + file;


            if((fen[i] >= 'A' && fen[i] <= 'Z') || (fen[i] >= 'a' && fen[i] <= 'z')) {
                set_bit(bitboards[char_pieces[fen[i]]], square);
            }

            else if(fen[i] >= '0' && fen[i] <= '9') {
                
                //difference in char values
                int offset = fen[i] - '0';
                // define piece variable
                int piece = -1;
                
                // loop over all piece bitboards
                for (int bb_piece = P; bb_piece <= k; bb_piece++)
                {
                    // if there is a piece on current square
                    if (is_occupied(bitboards[bb_piece], square))
                        // get piece code
                        piece = bb_piece;
                        break;
                }
                
                // on empty current square
                if (piece == -1)
                    // decrement file
                    file--;
                
                // adjust file counter
                file += offset;
            }

            else {
                file--;
            }
            
            i++;


        }
        
    }
    
    //side to mode
    i++;
    side = (fen[i] == 'w' ? white : black);

    //castling rights
    i += 2;
    while(fen[i] != ' ') {
        switch(fen[i]) {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
        }
        i++;
    }

    //en passant square
    i++;
    if(fen[i] != '-') {
        int file = fen[i] - 'a';
        i++;
        int rank = 8 - (fen[i] - '0');

        en_passant = rank*8 + file;
    }
    else {
        en_passant = no_sq;
    }

    update_occupancies();
}