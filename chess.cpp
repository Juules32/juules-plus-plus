
#include <iostream>
#include <string.h>
#include <map>
#include <chrono>
using namespace std;

// Defining custom type U64, consisting of 64 zeroes
#define U64 unsigned long long

// Bit manipulation macros, basically shorthands
#define is_occupied(bitboard, square) (bitboard & (1ULL << square))
#define get_bit(bitboard, square) ((bitboard & (1ULL << square)) ? 1 : 0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (is_occupied(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

// FEN dedug positions
char empty_board[] = "8/8/8/8/8/8/8/8 w - - ";
char start_position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
char pawns_position[] = "8/pppppppp/8/8/8/8/PPPPPPPP/8 w KQkq - 0 1 ";
char tricky_position[] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ";
char killer_position[] = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
char cmk_position[] = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 ";
char rook_position[] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";

// Names for white, black, rook and bishop
enum {white, black, both};
enum {rook, bishop};
enum {P, N, B, R, Q, K, p, n, b, r, q, k};
enum {wk = 1, wq = 2, bk = 4, bq = 8};

// Defining constant names to refer to corresponding index
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};

// Shorthand for getting the index of a square from its name
const char *index_to_square[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

// ASCII pieces
const char ascii_pieces[] = "PNBRQKpnbrqk";

// unicode pieces
const char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

map<char, int> char_pieces = {
    {'P', P},
    {'N', N},
    {'B', B},
    {'R', R},
    {'Q', Q},
    {'K', K},
    {'p', p},
    {'n', n},
    {'b', b},
    {'r', r},
    {'q', q},
    {'k', k}
};

map<char, int> promoted_pieces = {
    {Q, 'q'},
    {R, 'r'},
    {B, 'b'},
    {N, 'n'},
    {q, 'q'},
    {r, 'r'},
    {b, 'b'},
    {n, 'n'},
    {0, ' '}
};

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

// piece bitboards
U64 bitboards[12];
const int size_of_bitboards = sizeof(bitboards);

// occupancy bitboards
U64 occupancies[3];
const int size_of_occupancies = sizeof(occupancies);


// side to move
int side = -1;

// en_passant square
int en_passant = no_sq; 

// castling rights
int castle;





#define copy_board()                                                        \
    U64 bitboards_copy[12], occupancies_copy[3];                            \
    int side_copy, en_passant_copy, castle_copy;                            \
    memcpy(bitboards_copy, bitboards, size_of_bitboards);                   \
    memcpy(occupancies_copy, occupancies, size_of_occupancies);             \
    side_copy = side, en_passant_copy = en_passant, castle_copy = castle;   \

#define take_back()                                                         \
    memcpy(bitboards, bitboards_copy, size_of_bitboards);                    \
    memcpy(occupancies, occupancies_copy, size_of_occupancies);             \
    side = side_copy, en_passant = en_passant_copy, castle = castle_copy;   \


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
    int size = 0;
};

static inline void add_move(moves *move_list, int move) {
    move_list->array[move_list->size] = move;
    ++move_list->size;
}






unsigned int random_state = 1804289383;

// Generates a random 32-bit number from the current state
unsigned int rng_32()
{
    unsigned int number = random_state;

    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;

    // Updates state so new numbers can be found
    random_state = number;

    return number;
}

// Generates a random 64-bit number
U64 rng_64()
{

    // Generates four different 32-bit numbers where the first 16 bits are 0
    U64 n1 = (U64)(rng_32()) & 0xFFFF;
    U64 n2 = (U64)(rng_32()) & 0xFFFF;
    U64 n3 = (U64)(rng_32()) & 0xFFFF;
    U64 n4 = (U64)(rng_32()) & 0xFFFF;

    // Slices them all together
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// Generates a random 64-bit number with fewer 1's
U64 sparse_rng_64()
{
    return rng_64() & rng_64() & rng_64();
}

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

// Prints a bitboard
void print_bitboard(U64 bitboard)
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

void print_game() {
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

static inline void update_occupancies() {
    memset(occupancies, 0ULL, size_of_occupancies);

    for(int piece_type = P; piece_type <= K; piece_type++) {
        occupancies[white] |= bitboards[piece_type];
    }

    for(int piece_type = p; piece_type <= k; piece_type++) {
        occupancies[black] |= bitboards[piece_type];
    }

    occupancies[both] |= occupancies[white];
    occupancies[both] |= occupancies[black];
}

void parse_fen(char fen[]) {
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



// The U64 values for bitmaps used to isolate specific files
const U64 not_a = 18374403900871474942ULL;
const U64 not_ab = 18229723555195321596ULL;
const U64 not_h = 9187201950435737471ULL;
const U64 not_gh = 4557430888798830399ULL;
const U64 rank_2 = 71776119061217280ULL;
const U64 rank_7 = 65280ULL;

// Initializing empty attack arrays for the different pieces (pawns is 2d to account for color)
U64 pawn_attacks[2][64];
U64 pawn_quiet_moves[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 rook_masks[64];
U64 bishop_masks[64];
U64 rook_attacks[64][4096];
U64 bishop_attacks[64][512];

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

U64 rook_magic_numbers[64] = {
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
U64 bishop_magic_numbers[64] = {
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

static inline int is_square_attacked(int square, int side) {

    //The key point is that any piece (with the exception of pawns) can reach the same square it moved from

    //A pawn of opposite color should overlap with one of the white pawns' attacks
    if(side == white && (pawn_attacks[black][square] & bitboards[P])) return 1;
    if(side == black && (pawn_attacks[white][square] & bitboards[p])) return 1;
    if(knight_attacks[square] & ((side == white) ? bitboards[N] : bitboards[n])) return 1;
    if(king_attacks[square] & ((side == white) ? bitboards[K] : bitboards[k])) return 1;

    //Sliders rely on current occupancy
    if(get_bishop_attacks(square, occupancies[both]) & ((side == white) ? bitboards[B] : bitboards[b])) return 1;
    if(get_rook_attacks(square, occupancies[both]) & ((side == white) ? bitboards[R] : bitboards[r])) return 1;
    if(get_queen_attacks(square, occupancies[both]) & ((side == white) ? bitboards[Q] : bitboards[q])) return 1;

    return 0;
}

void print_attacked_squares(int side) {
    U64 result = 0ULL;
    for (int square = 0; square < 64; square++)
    {
        if(is_square_attacked(square, side)) set_bit(result, square);
    }
    
    print_bitboard(result);
}
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
        bitboard = bitboards[piece];
        
        // generate white pawns & white king castling moves
        if (side == white)
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
                    if (!(target_square < a8) && !get_bit(occupancies[both], target_square))
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
                            if ((source_square >= a2 && source_square <= h2) && !get_bit(occupancies[both], target_square - 8))
                                add_move(move_list, encode_move(source_square, target_square - 8, piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_attacks[side][source_square] & occupancies[black];
                    
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
                    if (en_passant != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with en_passant square (bit)
                        U64 en_passant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant);
                        
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
                if (castle & wk)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1))
                    {
                        // make sure king and the f1 squares are not under attacks
                        if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black))
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                // queen side castling is available
                if (castle & wq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1))
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
                    if (!(target_square > h1) && !get_bit(occupancies[both], target_square))
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
                            if ((source_square >= a7 && source_square <= h7) && !get_bit(occupancies[both], target_square + 8))
                                add_move(move_list, encode_move(source_square, target_square + 8, piece, 0, 0, 1, 0, 0));
                        }
                    }
                    
                    // init pawn attacks bitboard
                    attacks = pawn_attacks[side][source_square] & occupancies[white];
                    
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
                    if (en_passant != no_sq)
                    {
                        // lookup pawn attacks and bitwise AND with en_passant square (bit)
                        U64 en_passant_attacks = pawn_attacks[side][source_square] & (1ULL << en_passant);
                        
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
                if (castle & bk)
                {
                    // make sure square between king and king's rook are empty
                    if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8))
                    {
                        // make sure king and the f8 squares are not under attacks
                        if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white))
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1));
                    }
                }
                
                // queen side castling is available
                if (castle & bq)
                {
                    // make sure square between king and queen's rook are empty
                    if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8))
                    {
                        // make sure king and the d8 squares are not under attacks
                        if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white))
                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1));
                    }
                }
            }
        }
        
        // genarate knight moves
        if ((side == white) ? piece == N : piece == n)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = knight_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
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
        if ((side == white) ? piece == B : piece == b)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = get_bishop_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
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
        if ((side == white) ? piece == R : piece == r)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = get_rook_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
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
        if ((side == white) ? piece == Q : piece == q)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = get_queen_attacks(source_square, occupancies[both]) & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
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
        if ((side == white) ? piece == K : piece == k)
        {
            // loop over source squares of piece bitboard copy
            while (bitboard)
            {
                // init source square
                source_square = get_ls1b(bitboard);
                
                // init piece attacks in order to get set of target squares
                attacks = king_attacks[source_square] & ((side == white) ? ~occupancies[white] : ~occupancies[black]);
                
                // loop over target squares available from generated attacks
                while (attacks)
                {
                    // init target square
                    target_square = get_ls1b(attacks);    
                    
                    // quiet move
                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square))
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

static inline int make_move(int move, bool only_captures) {
    if(only_captures) {
        if(is_capture(move)) {
            make_move(move, false);
        }
        else return 0;
    }

    else {
        //reset the en passant square
        en_passant = no_sq;

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
        pop_bit(bitboards[piece], source);
        set_bit(bitboards[promotion_piece ? promotion_piece : piece], target);
        
        if(en_pass) {
            if(side == white) {
                pop_bit(bitboards[p], target + 8);
            }
            else {
                pop_bit(bitboards[P], target - 8);
            }
        }

        //if move is a capture, remove the attacked piece
        //important else, which saves running time
        else if(capture) {
            int start_piece, end_piece;
            if(side == white) {
                start_piece = p;
                end_piece = k;
            }
            else {
                start_piece = P;
                end_piece = K;
            }

            for (int bb_piece = start_piece; bb_piece < end_piece; bb_piece++)
            {
                if(is_occupied(bitboards[bb_piece], target)) {

                    pop_bit(bitboards[bb_piece], target);

                    break;
                }
            }
            
        }

        //set en passant square if a double pawn push was made
        //else is used to save time
        else if(double_pawn_push) {
            side == white ? en_passant = target + 8 :
                            en_passant = target - 8;
        }

        else if(castling) {
            switch (target) {
                case g1:
                    //if king side
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    break;
                
                case c1:
                    //if queen side
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    break;
                
                case g8:
                    //if king side
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    break;
                
                case c8:
                    //if queen side
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    break;
            }
        }

        //update castling rights
        castle &= castling_rights[source];
        castle &= castling_rights[target];

        //update occupancies
        update_occupancies();


        //check that the king is not in check
        if(is_square_attacked((side == white) ? get_ls1b(bitboards[K]) : get_ls1b(bitboards[k]), side ^ 1)) {
            take_back();
            //return illegal move
            return 0;
        }

        //switch sides
        side ^= 1;

        //return LEGAL move
        return 1;
    }
}

// Amount of reached nodes
long nodes;

// Recursive function to test how many possible positions exist
static inline void perft_driver(int depth) {

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
        perft_driver(depth-1);

        //retrieves the previous position
        take_back();
    }
}


void perft_test(int depth)
{
    printf("\n     Performance test\n\n");
    
    // Create move list instance
    moves move_list[1];
    
    // Generate moves
    generate_moves(move_list);
    
    // Init start time
    auto startTime = chrono::high_resolution_clock::now();
    
    // Loop over generated moves
    for (int move_count = 0; move_count < move_list->size; move_count++)
    {   
        // Preserve board state
        copy_board();
        
        // make move
        if (!make_move(move_list->array[move_count], false))
            // skip to the next move
            continue;
        
        // cummulative nodes
        long cummulative_nodes = nodes;
        
        // call perft driver recursively
        perft_driver(depth - 1);
        
        // old nodes
        long old_nodes = nodes - cummulative_nodes;
        
        // take back
        take_back();
        
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


int main()
{

    init_moves();
    parse_fen(tricky_position);

    perft_test(5);

    
    


    return 0;
}
