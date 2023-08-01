#pragma once
#include "shorthands.h"

/*
    The uci namespace is used for defining necessary
    universal chess interface functions    
*/

namespace uci {
    void init();
    void loop();
    int parse_position(string input);
    int parse_moves(string input);
    int parse_move(string move_string);
    int parse_go(string input);
    void print_engine_info();
}