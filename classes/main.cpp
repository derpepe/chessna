#include <thread>
#include "Uci.h"
#include "Engine.h"
#include "Comm.h"
#include "Board.h"
#include "Perft.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

void runTests() {
    struct PerftTest {
        std::string fen;
        std::vector<unsigned long long> nodes;
    };

    std::vector<PerftTest> tests;
    std::ifstream file("doc/perft.txt");
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }
        std::size_t pos = line.find(" -,");
        if (pos == std::string::npos) {
            continue;
        }
        PerftTest test;
        test.fen = line.substr(0, pos + 2) + " 0 1";
        std::string numbers = line.substr(pos + 3);
        std::stringstream ss(numbers);
        std::string num;
        while (std::getline(ss, num, ',')) {
            if (!num.empty()) {
                test.nodes.push_back(std::stoull(num));
            }
        }
        if (!test.nodes.empty()) {
            tests.push_back(test);
        }
    }

    bool all_tests_passed = true;
    Perft perft;
    int test_index = 0;
    for (const auto& test : tests) {
        ++test_index;
        Board board;
        board.loadFen(test.fen);
        for (std::size_t i = 0; i < test.nodes.size(); ++i) {
            int depth = static_cast<int>(i + 1);
            PerftResult result = perft.perft(board, depth);
            if (result.nodes != test.nodes[i]) {
                std::cout << "Test FAILED: Line " << test_index << " Depth: " << depth
                          << " Expected: " << test.nodes[i] << " Got: " << result.nodes << std::endl;
                all_tests_passed = false;
                break;
            }
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
