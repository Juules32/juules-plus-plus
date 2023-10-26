#include "shorthands.h"


U64 state::bitboards[12];

// Occupancy bitboards
U64 state::occupancies[3];

// Side to move
int state::side = -1;

// En_passant square
int state::en_passant = no_sq;

// Castling rights
int state::castle = 0;