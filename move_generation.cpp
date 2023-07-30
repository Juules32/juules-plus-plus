#include "move_generation.h"
#include "rng.h"
#include <iostream>
#include "string.h"



/*
                           castling   move     in      in
                              right update     binary  decimal

 king & rooks didn't move:     1111 & 1111  =  1111    15

        white king  moved:     1111 & 1100  =  1100    12
  white king's rook moved:     1111 & 1110  =  1110    14
 white queen's rook moved:     1111 & 1101  =  1101    13
     
         black king moved:     1111 & 0011  =  1011    3
  black king's rook moved:     1111 & 1011  =  1011    11
 black queen's rook moved:     1111 & 0111  =  0111    7

*/

// castling rights update constants
const int castling_rights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

U64 pawn_attacks[2][64];
 U64 pawn_quiet_moves[2][64];
 U64 knight_attacks[64];
 U64 king_attacks[64];
 U64 rook_masks[64];
 U64 bishop_masks[64];
 U64 rook_attacks[64][4096];
 U64 bishop_attacks[64][512];


void print_move(int move) {
    cout << "\n     "
        << index_to_square[get_source(move)]
        << index_to_square[get_target(move)]
        << char(promoted_pieces[get_promotion_piece(move)])
        << "\n";
}

void print_moves(moves *move_list) {

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






// The U64 values for bitmaps used to isolate specific files
const U64 not_a = 18374403900871474942ULL;
const U64 not_ab = 18229723555195321596ULL;
const U64 not_h = 9187201950435737471ULL;
const U64 not_gh = 4557430888798830399ULL;
const U64 rank_2 = 71776119061217280ULL;
const U64 rank_7 = 65280ULL;


// The amount of squares a bishop can move to, excluding the edge of the board, for each square index.
const int bishop_relevant_bits[] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6};

// The amount of squares a rook can move to, excluding the edge of the board, for each square index.
const int rook_relevant_bits[] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12};

const U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

// bishop magic numbers
const U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};

// Returns a bitboard of pawn attacks depending on side that excludes edge captures across board
U64 mask_pawn_attacks(bool side, int square)
{

    U64 attacks = 0ULL;
    U64 bitboard = 1ULL << square;

    // Remember - white is 0, black is 1
    if (side == white)
    {
        if (bitboard & not_h)
            attacks |= bitboard >> 7;
        if (bitboard & not_a)
            attacks |= bitboard >> 9;
    }
    else
    {
        if (bitboard & not_h)
            attacks |= bitboard << 9;
        if (bitboard & not_a)
            attacks |= bitboard << 7;
    }

    return attacks;
}

U64 mask_pawn_quiet_moves(bool side, int square)
{

    U64 moves = 0ULL;
    U64 bitboard = 1ULL << square;

    // Remember - white is 0, black is 1
    if (side == white)
    {
        if (bitboard & rank_2)
            moves |= bitboard >> 16;

        moves |= bitboard >> 8;
    }
    else
    {
        if (bitboard & rank_7)
            moves |= bitboard << 16;

        moves |= bitboard << 8;
    }

    return moves;
}

// Returns a bitboard of knight attacks that excludes moving across the board
U64 mask_knight_attacks(int square)
{
    U64 attacks = 0ULL;
    U64 bitboard = 1ULL << square;

    if ((bitboard >> 6) & not_ab)
        attacks |= bitboard >> 6;
    if ((bitboard >> 10) & not_gh)
        attacks |= bitboard >> 10;
    if ((bitboard >> 15) & not_a)
        attacks |= bitboard >> 15;
    if ((bitboard >> 17) & not_h)
        attacks |= bitboard >> 17;
    if ((bitboard << 6) & not_gh)
        attacks |= bitboard << 6;
    if ((bitboard << 10) & not_ab)
        attacks |= bitboard << 10;
    if ((bitboard << 15) & not_h)
        attacks |= bitboard << 15;
    if ((bitboard << 17) & not_a)
        attacks |= bitboard << 17;

    return attacks;
}

// Returns a bitboard of king attacks that excludes moving across the board
U64 mask_king_attacks(int square)
{
    U64 attacks = 0ULL;
    U64 bitboard = 1ULL << square;

    attacks |= bitboard >> 8;
    attacks |= bitboard << 8;
    if (bitboard & not_a)
    {
        attacks |= bitboard >> 1;
        attacks |= bitboard >> 9;
        attacks |= bitboard << 7;
    }
    if (bitboard & not_h)
    {
        attacks |= bitboard << 1;
        attacks |= bitboard << 9;
        attacks |= bitboard >> 7;
    }

    return attacks;
}

// Returns a bitboard of bishop attacks by going in each direction until an edge is reached
U64 mask_bishop_attacks(int square)
{
    U64 attacks = 0ULL;

    int tr, tf;
    tr = square / 8;
    tf = square % 8;

    for (int r = tr + 1, f = tf + 1; r < 7 && f < 7; r++, f++)
        attacks |= 1ULL << r * 8 + f;
    for (int r = tr + 1, f = tf - 1; r < 7 && f > 0; r++, f--)
        attacks |= 1ULL << r * 8 + f;
    for (int r = tr - 1, f = tf + 1; r > 0 && f < 7; r--, f++)
        attacks |= 1ULL << r * 8 + f;
    for (int r = tr - 1, f = tf - 1; r > 0 && f > 0; r--, f--)
        attacks |= 1ULL << r * 8 + f;

    return attacks;
}

// Returns a bitboard of rook attacks by going in each direction until an edge is reached
U64 mask_rook_attacks(int square)
{
    U64 attacks = 0ULL;

    int tr, tf;
    tr = square / 8;
    tf = square % 8;

    for (int r = tr + 1; r < 7; r++)
        attacks |= 1ULL << r * 8 + tf;
    for (int r = tr - 1; r > 0; r--)
        attacks |= 1ULL << r * 8 + tf;
    for (int f = tf + 1; f < 7; f++)
        attacks |= 1ULL << tr * 8 + f;
    for (int f = tf - 1; f > 0; f--)
        attacks |= 1ULL << tr * 8 + f;

    return attacks;
}

// Returns a bitboard of bishop attacks, depending on whether a blocker is found
U64 bishop_moves_on_the_fly(int square, U64 blockers)
{
    U64 attacks = 0ULL;

    int tr = square / 8;
    int tf = square % 8;

    for (int r = tr + 1, f = tf + 1; r < 8 && f < 8; r++, f++)
    {
        attacks |= 1ULL << r * 8 + f;
        if ((1ULL << r * 8 + f) & blockers)
            break;
    }
    for (int r = tr + 1, f = tf - 1; r < 8 && f >= 0; r++, f--)
    {
        attacks |= 1ULL << r * 8 + f;
        if ((1ULL << r * 8 + f) & blockers)
            break;
    }
    for (int r = tr - 1, f = tf + 1; r >= 0 && f < 8; r--, f++)
    {
        attacks |= 1ULL << r * 8 + f;
        if ((1ULL << r * 8 + f) & blockers)
            break;
    }
    for (int r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
    {
        attacks |= 1ULL << r * 8 + f;
        if ((1ULL << r * 8 + f) & blockers)
            break;
    }

    return attacks;
}

// Returns a bitboard of rook attacks, depending on whether a blocker is found
U64 rook_moves_on_the_fly(int square, U64 blockers)
{
    U64 attacks = 0ULL;

    int tr = square / 8;
    int tf = square % 8;

    for (int r = tr + 1; r < 8; r++)
    {
        attacks |= 1ULL << r * 8 + tf;
        if ((1ULL << r * 8 + tf) & blockers)
            break;
    }
    for (int r = tr - 1; r >= 0; r--)
    {
        attacks |= 1ULL << r * 8 + tf;
        if ((1ULL << r * 8 + tf) & blockers)
            break;
    }
    for (int f = tf + 1; f < 8; f++)
    {
        attacks |= 1ULL << tr * 8 + f;
        if ((1ULL << tr * 8 + f) & blockers)
            break;
    }
    for (int f = tf - 1; f >= 0; f--)
    {
        attacks |= 1ULL << tr * 8 + f;
        if ((1ULL << tr * 8 + f) & blockers)
            break;
    }

    return attacks;
}


// Generates the appropriate bitboard from a permutation and attack_mask
U64 set_occupancy(int permutation, int num_bits, U64 attack_mask)
{
    U64 occupancy = 0ULL;
    int square;
    for (int count = 0; count < num_bits; count++)
    {
        square = get_ls1b(attack_mask);
        pop_bit(attack_mask, square);
        if (permutation & (1 << count))
            set_bit(occupancy, square);
    }
    return occupancy;
}

// Initializes the different non-sliding piece attacks
void init_leaper_moves()
{
    for (int square = 0; square < 64; square++)
    {
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_quiet_moves[white][square] = mask_pawn_quiet_moves(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
        pawn_quiet_moves[black][square] = mask_pawn_quiet_moves(black, square);
        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
    }
}

void init_slider_attacks(bool bishop) {
    for (int square = 0; square < 64; square++)
    {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];

        int relevant_bits = count_bits(attack_mask);

        int max_occupancy_index = 1 << relevant_bits;

        for (int i = 0; i < max_occupancy_index; i++)
        {
            if (bishop) {
                U64 occupancy = set_occupancy(i, relevant_bits, attack_mask);

                int magic_index = (occupancy * bishop_magic_numbers[square] >> (64 - bishop_relevant_bits[square]));

                bishop_attacks[square][magic_index] = bishop_moves_on_the_fly(square, occupancy);
            }

            else {
                U64 occupancy = set_occupancy(i, relevant_bits, attack_mask);

                int magic_index = (occupancy * rook_magic_numbers[square] >> (64 - rook_relevant_bits[square]));

                rook_attacks[square][magic_index] = rook_moves_on_the_fly(square, occupancy);
            }
        }
        
    }
    
}

void init_moves() {
    init_leaper_moves();
    init_slider_attacks(bishop);
    init_slider_attacks(rook);
}


// Generates a valid magic number for a square and sliding piece type
U64 find_magic_number(int square, int relevant_bits, int bishop)
{

    // There can be at max 2^12 = 4096 different permutations of occupancies
    U64 occupancies[4096];

    U64 attacks[4096];

    U64 used_attacks[4096];

    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

    // This is used since the maximum index varies depending on how many squares a given piece can move to
    int max_occupancy_index = 1 << relevant_bits;
    for (int i = 0; i < max_occupancy_index; i++)
    {
        occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);

        attacks[i] = bishop ? 
            bishop_moves_on_the_fly(square, occupancies[i]) : 
            rook_moves_on_the_fly(square, occupancies[i]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        U64 magic_number = sparse_rng_64();

        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        bool failed = false;
        for (int i = 0; i < max_occupancy_index && !failed; i++)
        {
            int magic_index = (int)((occupancies[i] * magic_number) >> (64 - relevant_bits));

            if (used_attacks[magic_index] == 0ULL)
                used_attacks[magic_index] = attacks[i];

            else if (used_attacks[magic_index] != attacks[i])
                failed = true;
        }

        if (!failed)
            return magic_number;
    }

    cout << "No magic number could be found\n";
    return 0ULL;
}


void init_magic_numbers()
{
    for (int square = 0; square < 64; square++)
    {
        printf(" 0x%llxUll,\n", find_magic_number(square, rook_relevant_bits[square], rook));
    }

    cout << "\n\n";

    for (int square = 0; square < 64; square++)
    {
        printf(" 0x%llxUll,\n", find_magic_number(square, bishop_relevant_bits[square], bishop));
    }
}