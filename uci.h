#pragma once
#include "shorthands.h"

/*
    The uci namespace is used for defining necessary
    universal chess interface functions    
*/

namespace uci {
    void init();
    void loop();
    int parse_move(string move_string);
    int parse_moves(string input);
}