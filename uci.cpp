#include "uci.h"
#include "board.h"
#include "utils.h"
#include "perft.h"
#include <sstream>
#include <vector>

void uci::init()
{
    uci::print_engine_info();
    uci::loop();
}

void uci::print_engine_info() {
    cout << "id name ChessPlusPlus" << endl;
    cout << "id author Juules32" << endl;
    cout << "uciok\n" << endl;
}

void uci::loop()
{
    string input;
    while (true)
    {
        getline(cin, input);

        if (input == "quit" || input == "exit")
        {
            break; // Exit the loop if the user enters "quit"
        }

        else if (input == "uci") {
            uci::print_engine_info();
        }

        else if (input == "isready") {
            cout << "readyok" << endl;
        }

        else if (input == "ucinewgame") {
            uci::parse_position("position startpos");
        }

        uci::parse_position(input);
        uci::parse_go(input);
    }
}

int uci::parse_position(string input)
{
    int position_i = input.find("position");

    if (position_i != string::npos)
    {
        int startpos_i = input.find("startpos");
        int fen_i = input.find("fen");
        int moves_i = input.find("moves");
        // Get position
        if (startpos_i != string::npos)
        {
            board::parse_fen(start_position);
        }
        else if (fen_i != string::npos)
        {
            board::parse_fen(input.substr(fen_i + 4));
        }

        // Make moves if specified
        if (moves_i != string::npos)
        {
            uci::parse_moves(input.substr(moves_i + 6));
        }

        print::game();
    }
}

int uci::parse_go(string input) {
    int go_i = input.find("go");

    if (go_i != string::npos)
    {
        //demo
        cout << "bestmove d2d4" << endl;
        int depth_i = input.find("depth");
        int perft_i = input.find("perft");

        if (depth_i != string::npos) {
            int depth = stoi(input.substr(depth_i + 6));
        }
        else if (perft_i != string::npos) {
            // String to integer
            perft::test(stoi(input.substr(perft_i + 6)));
        }

    }
}

int uci::parse_moves(string input)
{

    // Creates a stringstream from the input string
    stringstream ss(input);

    // Uses a vector to store the substrings
    vector<string> substrings;

    string substring;

    // Extracts substrings separated by space and stores them in the vector
    while (ss >> substring)
        substrings.push_back(substring);

    for (const string &str : substrings)
    {
        if (uci::parse_move(str))
            board::make_move(uci::parse_move(str));
    }
}

int uci::parse_move(string move_string)
{

    int source_square = move_string[0] - 'a' + (8 - (move_string[1] - '0')) * 8;
    int target_square = move_string[2] - 'a' + (8 - (move_string[3] - '0')) * 8;

    moves move_list[1];

    board::generate_moves(move_list);

    for (int move_count = 0; move_count < move_list->size; move_count++)
    {
        int current_move = move_list->array[move_count];
        if (source_square == get_source(current_move) && target_square == get_target(current_move))
        {
            int promotion_piece = get_promotion_piece(current_move) % 6;
            if (!promotion_piece)
                return current_move;

            switch (move_string[4])
            {
            case 'q':
                if (promotion_piece == Q)
                    return current_move;
                else
                    return 0;
                break;

            case 'r':
                if (promotion_piece == R)
                    return current_move;
                else
                    return 0;
                break;

            case 'b':
                if (promotion_piece == B)
                    return current_move;
                else
                    return 0;
                break;

            case 'n':
                if (promotion_piece == N)
                    return current_move;
                else
                    return 0;
                break;

            default:
                return 0;
                break;
            }
        }
    }

    return 0;
}