#include "main.cpp"

extern "C" const char* test(const char* test_string) {
    return test_string;
}

extern "C" void setup() {
    move_gen::init();
    parse::fen(start_position);
}

extern "C" void print_game() {
    print::game();
}

extern "C" int make_move(const char* move_string) {
    cout << "making move: " << move_string << endl;
    int move = uci::parse_move(move_string);
    cout << move << endl;
    int success = move_exec::make_move(move);
    print_game();
    return success;
}

extern "C" void find_move() {
    move_exec::search_position(6);
    int move = move_exec::pv_table[0][0];
    move_exec::make_move(move);
    print_game();
}
