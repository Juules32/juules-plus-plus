# juules-plus-plus
This is the github repository for the chess engine, JuulesPlusPlus. This is a passion project of mine that I created with the goal of making an engine that could beat me regularly in the game of chess. It certainly can, primarily because of its relatively high calculation speed. Where it currently lacks precision is in the endgame, where some key concepts are missing.

JuulesPlusPlus supports Universal Chess Interface (UCI) integration, so I made it into a [Lichess bot](https://lichess.org/@/JuulesPlusPlus)! This bot will in all likeliness be offline since I have to run the bot locally, but in the rare event that it *is* online, feel free to send a challenge ☺. Another way to play against the engine is to download the free software [Arena Chess GUI](http://www.playwitharena.de/).

## How to run
There are two main ways to run this engine.
1. The simple way:
- Find the latest [release](https://github.com/Juules32/JuulesPlusPlus/releases) and download the version for your operating system. 
- Extract and run the executable.
2. For more control:
- Download or clone the repository.
- Open your terminal of choice and navigate to the root folder.
- Run `make publish` to compile an executable program. This requires a C++ compiler (such as [GNU](https://gcc.gnu.org/) to be installed). The executable will be placed in the `bin` folder by default but can be modified with the command: `make publish OUTPUT_DIR="<path_name>"`.
- Run the executable.

## How to use
Once you have your executable running, the main way to interact with the engine is through UCI commands:

1. `[quit | exit]`
- Terminates the program.

2. `ucinewgame`
- Shorthand for `position startpos`, which starts a new game.

3. `position [<preset> | <fen_string>] [moves <move1> <move2> ...]`
- Used to set the position to the given input.
- Presets include: `startpos`, `trickypos`, `killerpos`, and others.
- A [`<fen_string>`](https://www.chessprogramming.org/Forsyth-Edwards_Notation) can be used instead (without quotation marks).
- Then comes a list of `moves` to make after setting the position. The moves must be written in the form `<start_square><end_square>`.
- For example, a valid command could be: `position startpos moves e2e4 e7e5 g1f3`, which sets up the starting position and goes into the [King's Knight Variation](https://www.chess.com/openings/Kings-Pawn-Opening-Kings-Knight-Variation).

4. `go depth <plies>`
- Determines and returns the best move N plies (half-moves) into the future.

5. `go perft <plies>` 
- Used for debugging and performance purposes. For each available move, it displays the number of possible game states N plies into the future.

## Special Thanks
I want to shout out the amazing [Code Monkey King](https://github.com/maksimKorzh), whose [series on bitboard chess engines](https://www.youtube.com/watch?v=QUNP-UjujBM&list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs) was immensely useful for the development of this project. Please do check out and support his work!
