#include "main.cpp"

extern "C" const char* setup() {
    move_gen::init();
    parse::fen(start_position);
    return format::game_fen().c_str();
}

extern "C" void print_game() {
    print::game();
}

extern "C" const char* engine_move() {
    move_exec::search_position(6);
    int move = move_exec::pv_table[0][0];
    move_exec::make_move(move);
    print_game();
    return format::game_fen().c_str();
}

extern "C" U64 valid_targets(int player_source, int player_side) {
    U64 result = 0ULL;

    moves move_list[1];
    move_gen::generate_moves(move_list);

    copy_state();
    
    for (size_t i = 0; i < move_list->size; i++) {
        int current_move = move_list->array[i];
        int source = get_source(current_move);
        int piece = get_piece(current_move);
        int side = (piece >= P && piece <= K) ? white : black;
        if (source == player_source && (side == player_side || player_side == both)) {
            if (move_exec::make_move(current_move)) {
                set_bit(result, get_target(current_move));
            }
            print::bitboard(state::occupancies[both]);
            revert_state();
        }
    }

    return result;
}

extern "C" int valid_move(int player_source, int player_target, int player_side) {
    moves move_list[1];
    move_gen::generate_moves(move_list);

    for (size_t i = 0; i < move_list->size; i++) {
        int current_move = move_list->array[i];
        int source = get_source(current_move);
        int target = get_target(current_move);
        int piece = get_piece(current_move);
        int side = (piece >= P && piece <= K) ? white : black;
        if (source == player_source && target == player_target && (side == player_side || player_side == both)) {
            return current_move;
        }
    }
    return 0;
}

extern "C" const char* make_move(int move) {
    int success = move_exec::make_move(move);
    print_game();
    return format::game_fen().c_str();
}

extern "C" const char* make_move_str(const char* move_string) {
    return make_move(uci::parse_move(move_string));
}
