#pragma once
#include <string>

/*
    The uci namespace is used for defining necessary
    universal chess interface functions    
*/

namespace uci {
    void init();
    void loop();
    void parse_position(std::string input);
    void parse_moves(std::string input);
    int parse_move(std::string move_string);
    void parse_go(std::string input);
    void print_engine_info();
}
