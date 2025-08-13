# CHESSna 2

CHESSna 2 is a simple chess engine written in C++. It uses the Universal Chess Interface (UCI) protocol for communication.

## Specialties

This version of CHESSna 2 includes a custom `perft` command for testing the move generation.

## Building

To build the engine, you need a C++ compiler that supports C++11. Then, simply run `make`:

```
make
```

This will create the `chessna` executable in the `bin` directory.

## Running

You can run the engine from the command line:

```
./bin/chessna
```

The engine will then wait for UCI commands.

## Commands

### Standard UCI Commands

The engine supports the following standard UCI commands:

- `uci`: Print engine information.
- `isready`: Check if the engine is ready.
- `position [fen <fenstring> | startpos] moves <move1> ... <moveN>`: Set the board position.
- `go`: Start calculating the best move.
- `stop`: Stop calculating.
- `quit`: Quit the engine.

### Custom Commands

#### `perft <depth>`

This command performs a performance test of the move generator. It counts the number of legal moves from the current position to a given depth.

Example:

```
perft 5
```

This will calculate the number of legal moves up to a depth of 5 and print the result.
