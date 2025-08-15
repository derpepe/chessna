#include <thread>
#include "Uci.h"
#include "Engine.h"
#include "Comm.h"
#include "Board.h"
#include "Perft.h"
#include <iostream>
#include <vector>
#include <string>

void runTests() {
    struct PerftTest {
        std::string fen;
        int depth;
        unsigned long long expected_nodes;
    };

    std::vector<PerftTest> tests = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 1, 48},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2, 2039},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 1, 14},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 2, 191},
    };

    bool all_tests_passed = true;
    Perft perft;
    for (const auto& test : tests) {
        Board board;
        board.loadFen(test.fen);
        PerftResult result = perft.perft(board, test.depth);
        if (result.nodes == test.expected_nodes) {
            std::cout << "Test passed: FEN: " << test.fen << " Depth: " << test.depth << " Result: " << result.nodes << std::endl;
        } else {
            std::cout << "Test FAILED: FEN: " << test.fen << " Depth: " << test.depth << " Expected: " << test.expected_nodes << " Got: " << result.nodes << std::endl;
            all_tests_passed = false;
        }
    }

    if (all_tests_passed) {
        std::cout << "All perft tests passed!" << std::endl;
    } else {
        std::cout << "Some perft tests failed." << std::endl;
    }
}

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--test") {
        runTests();
        return 0;
    }

	Comm *comm = new Comm();
	
	Uci *uci = new Uci(comm);
	Engine *engine = new Engine(comm);

	// initialize asynchronous communication
	std::thread uci_thread( [uci] { uci->run(); } );
	std::thread engine_thread( [engine] { engine->run(); } );

	// wait for both threads
	uci_thread.join();
	engine_thread.join();
	return 0;
}
