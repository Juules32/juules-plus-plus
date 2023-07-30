#pragma once
#include "shorthands.h"
#include "board.h"


/*
          binary move bits                               hexidecimal constants
    
    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    en passant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/



//Macro to encode move
#define encode_move(source, target, piece, promoted, capture, double_pawn_push, en_passant, castling) \
    (source) |                  \
    (target << 6) |             \
    (piece << 12) |             \
    (promoted << 16) |          \
    (capture << 20) |           \
    (double_pawn_push << 21) |  \
    (en_passant << 22) |        \
    (castling << 23)            \

//Macros to extract move information 
#define get_source(move) (move & 0x3f)
#define get_target(move) ((move & 0xfc0) >> 6)
#define get_piece(move) ((move & 0xf000) >> 12)
#define get_promotion_piece(move) ((move & 0xf0000) >> 16)
#define is_capture(move) (move & 0x100000)
#define is_double_pawn_push(move) (move & 0x200000)
#define is_en_passant(move) (move & 0x400000)
#define is_castling(move) (move & 0x800000)


struct moves {
    int array[256];
    int size;
};

static inline void add_move(moves *move_list, int move) {
    move_list->array[move_list->size] = move;
    ++move_list->size;
}

extern const int castling_rights[64];


void print_move(int move);

void print_moves(moves *move_list);


// Initializing empty attack arrays for the different pieces (pawns is 2d to account for color)
extern U64 pawn_attacks[2][64];
extern U64 pawn_quiet_moves[2][64];
extern U64 knight_attacks[64];
extern U64 king_attacks[64];
extern U64 rook_masks[64];
extern U64 bishop_masks[64];
extern U64 rook_attacks[64][4096];
extern U64 bishop_attacks[64][512];




// The amount of squares a bishop can move to, excluding the edge of the board, for each square index.
extern const int bishop_relevant_bits[];

// The amount of squares a rook can move to, excluding the edge of the board, for each square index.
extern const int rook_relevant_bits[];

extern const U64 rook_magic_numbers[64];
// bishop magic numbers
extern const U64 bishop_magic_numbers[64];


// Counts the amount of bits (1's) in a given bitboard
static inline int count_bits(U64 bitboard)
{
    int count = 0;

    while (bitboard)
    {
        count++;
        bitboard &= bitboard - 1;
    }

    return count;
}

// Returns the least significant first bit in a bitboard (going from the top left corner)
// The bitboard returned consists of 1's until the first 1 in the given bitboard is found
static inline int get_ls1b(U64 bitboard)
{
    if (!bitboard)
        return -1;
    return count_bits((bitboard & -bitboard) - 1);
}

void init_moves();


//Faster version of bishop_moves_on_the_fly that uses magic numbers
static inline U64 get_bishop_attacks(int square, U64 occupancy) {
    
    //Filters out pieces the piece cannot move to    
    occupancy &= bishop_masks[square];

    //Occupancy becomes magic index for bishop attack retrieval by using corresponding magic number
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];

    return bishop_attacks[square][occupancy];
}

//Rook version
static inline U64 get_rook_attacks(int square, U64 occupancy) {
    
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    return rook_attacks[square][occupancy];
}

static inline U64 get_queen_attacks(int square, U64 occupancy) {

    //Simply combines possible rook and bishop moves
    return get_rook_attacks(square, occupancy) | get_bishop_attacks(square, occupancy);
}


static inline int is_square_attacked(int square, int side) {

    //The key point is that any piece (with the exception of pawns) can reach the same square it moved from

    //A pawn of opposite color should overlap with one of the white pawns' attacks
    if(side == white && (pawn_attacks[black][square] & board::bitboards[P])) return 1;
    if(side == black && (pawn_attacks[white][square] & board::bitboards[p])) return 1;
    if(knight_attacks[square] & ((side == white) ? board::bitboards[N] : board::bitboards[n])) return 1;
    if(king_attacks[square] & ((side == white) ? board::bitboards[K] : board::bitboards[k])) return 1;

    //Sliders rely on current occupancy
    if(get_bishop_attacks(square, board::occupancies[both]) & ((side == white) ? board::bitboards[B] : board::bitboards[b])) return 1;
    if(get_rook_attacks(square, board::occupancies[both]) & ((side == white) ? board::bitboards[R] : board::bitboards[r])) return 1;
    if(get_queen_attacks(square, board::occupancies[both]) & ((side == white) ? board::bitboards[Q] : board::bitboards[q])) return 1;

    return 0;
}


void print_attacked_squares(int side);


// generate all moves
static inline void generate_moves(moves *move_list)
{
    // init move count
    move_list->size = 0;

    // define source & target squares
    int source_square, target_square;
    
    // define current piece's bitboard copy & it's attacks
    U64 bitboard, attacks;
    
    // loop over all the bitboards
    for (int piece = P; piece <= k; piece++)
    {
        // init piece bitboard copy
        bitboard = board::bitboards[piece];
        
        // generate white pawns & white king castling moves
        if (board::side == white)
        {
            // pick up white pawn bitboards index
            if (piece == P)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b(bitboard);
                    
                    // init target square
                    target_square = source_square - 8;
                    
                    // generate quiet pawn moves
                    if (!(target_square < a8) && !get_bit(board::occupancies[both], target_square))
                    {
                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {                            
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            // two squares ahead pawn move
                            if ((source_square >= a2 && source_square <= h2) && !get_bit(board::occupancies[both], target_square - 8))
                                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_attacks[board::side][source_square] & board::occupancies[black];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b(attacks);
                        
                        // pawn promotion
                        if (source_square >= a7 && source_square <= h7)
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0));
                        }
                        
                        else
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }
                    
                    // generate en_passant captures
                    if (board::en_passant != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with en_passant square (bit)
                        U64 en_passant_attacks = pawn_attacks[board::side][source_square] & (1ULL << board::en_passant);
                        
                        // make sure en_passant capture available
                        if (en_passant_attacks)
                        {
                            // init en_passant capture target square
                            int target_en_passant = get_ls1b(en_passant_attacks);
                            add_move(move_list, encode_move(source_square, target_en_passant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }
            
            // castling moves
            if (piece == K)
            {
                // king side castling is available
                if (board::castle & wk)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(board::occupancies[both], f1) && !get_bit(board::occupancies[both], g1))
                    {
                        // make sure king and the f1 squares are not under attacks
                        if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                // queen side castling is available
                if (board::castle & wq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(board::occupancies[both], d1) && !get_bit(board::occupancies[both], c1) && !get_bit(board::occupancies[both], b1))
                    {
                        // make sure king and the d1 squares are not under attacks
                        if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black))
                            add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        
        // generate black pawns & black king castling moves
        else
        {
            // pick up black pawn bitboards index
            if (piece == p)
            {
                // loop over white pawns within white pawn bitboard
                while (bitboard)
                {
                    // init source square
                    source_square = get_ls1b(bitboard);
                    
                    // init target square
                    target_square = source_square + 8;
                    
                    // generate quiet pawn moves
                    if (!(target_square > h1) && !get_bit(board::occupancies[both], target_square))
                    {
                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0));
                        }
                        
                        else
                        {
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            
                            // two squares ahead pawn move
                            if ((source_square >= a7 && source_square <= h7) && !get_bit(board::occupancies[both], target_square + 8))
                                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_attacks[board::side][source_square] & board::occupancies[white];
                    
                    // generate pawn captures
                    while (attacks)
                    {
                        // init target square
                        target_square = get_ls1b(attacks);
                        
                        // pawn promotion
                        if (source_square >= a2 && source_square <= h2)
                        {
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0));
                        }
                        
                        else
                            // one square ahead pawn move
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                        
                        // pop ls1b of the pawn attacks
                        pop_bit(attacks, target_square);
                    }
                    
                    // generate en_passant captures
                    if (board::en_passant != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with en_passant square (bit)
                        U64 en_passant_attacks = pawn_attacks[board::side][source_square] & (1ULL << board::en_passant);
                        
                        // make sure en_passant capture available
                        if (en_passant_attacks)
                        {
                            // init en_passant capture target square
                            int target_en_passant = get_ls1b(en_passant_attacks);
                            add_move(move_list, encode_move(source_square, target_en_passant, piece, 0, 1, 0, 1, 0));
                        }
                    }
                    
                    // pop ls1b from piece bitboard copy
                    pop_bit(bitboard, source_square);
                }
            }
            
            // castling moves
            if (piece == k)
            {
                // king side castling is available
                if (board::castle & bk)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(board::occupancies[both], f8) && !get_bit(board::occupancies[both], g8))
                    {
                        // make sure king and the f8 squares are not under attacks
                        if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                // queen side castling is available
                if (board::castle & bq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(board::occupancies[both], d8) && !get_bit(board::occupancies[both], c8) && !get_bit(board::occupancies[both], b8))
                    {
                        // make sure king and the d8 squares are not under attacks
                        if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        
        // genarate knight moves
        if ((board::side == white) ? piece == N : piece == n)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = knight_attacks[source_square] & ((board::side == white) ? ~board::occupancies[white] : ~board::occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((board::side == white) ? board::occupancies[black] : board::occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        
        // generate bishop moves
        if ((board::side == white) ? piece == B : piece == b)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = get_bishop_attacks(source_square, board::occupancies[both]) & ((board::side == white) ? ~board::occupancies[white] : ~board::occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((board::side == white) ? board::occupancies[black] : board::occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        
        // generate rook moves
        if ((board::side == white) ? piece == R : piece == r)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = get_rook_attacks(source_square, board::occupancies[both]) & ((board::side == white) ? ~board::occupancies[white] : ~board::occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((board::side == white) ? board::occupancies[black] : board::occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
        
        // generate queen moves
        if ((board::side == white) ? piece == Q : piece == q)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = get_queen_attacks(source_square, board::occupancies[both]) & ((board::side == white) ? ~board::occupancies[white] : ~board::occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((board::side == white) ? board::occupancies[black] : board::occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }
                
                
                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }

        // generate king moves
        if ((board::side == white) ? piece == K : piece == k)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = king_attacks[source_square] & ((board::side == white) ? ~board::occupancies[white] : ~board::occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((board::side == white) ? board::occupancies[black] : board::occupancies[white]), target_square))
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                    
                    else
                        // capture move
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));
                    
                    // pop ls1b in current attacks set
                    pop_bit(attacks, target_square);
                }

                // pop ls1b of the current piece bitboard copy
                pop_bit(bitboard, source_square);
            }
        }
    }
}


static inline int make_move(int move, bool only_captures = false) {
    if(only_captures) {
        if(is_capture(move)) {
            make_move(move, false);
        }
        else return 0;
    }

    else {
        //reset the en passant square
        board::en_passant = no_sq;

        copy_board();

        int source = get_source(move);
        int target = get_target(move);
        int piece = get_piece(move);
        int promotion_piece = get_promotion_piece(move);
        int capture = is_capture(move);
        int double_pawn_push = is_double_pawn_push(move);
        int en_pass = is_en_passant(move);
        int castling = is_castling(move);


        //move piece
        pop_bit(board::bitboards[piece], source);
        set_bit(board::bitboards[promotion_piece ? promotion_piece : piece], target);
        
        if(en_pass) {
            if(board::side == white) {
                pop_bit(board::bitboards[p], target + 8);
            }
            else {
                pop_bit(board::bitboards[P], target - 8);
            }
        }

        //if move is a capture, remove the attacked piece
        //important else, which saves running time
        else if(capture) {
            int start_piece, end_piece;
            if(board::side == white) {
                start_piece = p;
                end_piece = k;
            }
            else {
                start_piece = P;
                end_piece = K;
            }

            for (int bb_piece = start_piece; bb_piece < end_piece; bb_piece++)
            {
                if(is_occupied(board::bitboards[bb_piece], target)) {

                    pop_bit(board::bitboards[bb_piece], target);

                    break;
                }
            }
            
        }

        //set en passant square if a double pawn push was made
        //else is used to save time
        else if(double_pawn_push) {
            board::side == white ? board::en_passant = target + 8 :
                            board::en_passant = target - 8;
        }

        else if(castling) {
            switch (target) {
                case g1:
                    //if king side
                    pop_bit(board::bitboards[R], h1);
                    set_bit(board::bitboards[R], f1);
                    break;
                
                case c1:
                    //if queen side
                    pop_bit(board::bitboards[R], a1);
                    set_bit(board::bitboards[R], d1);
                    break;
                
                case g8:
                    //if king side
                    pop_bit(board::bitboards[r], h8);
                    set_bit(board::bitboards[r], f8);
                    break;
                
                case c8:
                    //if queen side
                    pop_bit(board::bitboards[r], a8);
                    set_bit(board::bitboards[r], d8);
                    break;
            }
        }

        //update castling rights
        board::castle &= castling_rights[source];
        board::castle &= castling_rights[target];

        //update occupancies
        board::update_occupancies();


        //check that the king is not in check
        if(is_square_attacked((board::side == white) ? get_ls1b(board::bitboards[K]) : get_ls1b(board::bitboards[k]), board::side ^ 1)) {
            take_back();
            //return illegal move
            return 0;
        }

        //switch sides
        board::side ^= 1;

        //return LEGAL move
        return 1;
    }
}