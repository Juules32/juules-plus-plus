#pragma once
#include <string>

/*
    The uci namespace is used for defining necessary
    universal chess interface functions    
*/

namespace uci {
    int parse_move(std::string move_string);
}