#include <thread>
#include "Uci.h"
#include "Engine.h"
#include "Comm.h"
#include "Board.h"
#include "Perft.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include "perft_data.h"

void runTests(bool deepTest) {
    struct PerftTest {
        std::string fen;
        std::vector<unsigned long long> nodes;
    };

    std::vector<PerftTest> tests;
    for (size_t i = 0; i < PERFT_DATA_COUNT; ++i) {
        std::string line(PERFT_DATA[i]);
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
        std::cout << "Running test " << test_index << ": " << test.fen << std::endl;

        Board board;
        board.loadFen(test.fen);

        bool test_passed = true;
        std::size_t max_depth = deepTest ? test.nodes.size() : std::min<std::size_t>(4, test.nodes.size());
        for (std::size_t i = 0; i < max_depth; ++i) {
            int depth = static_cast<int>(i + 1);
            PerftResult result = perft.perft(board, depth);
            bool depth_passed = (result.nodes == test.nodes[i]);

            std::cout << "  Depth " << depth << ": expected " << test.nodes[i]
                      << ", got " << result.nodes
                      << (depth_passed ? " [OK]" : " [FAIL]") << std::endl;

            if (!depth_passed) {
                all_tests_passed = false;
                test_passed = false;
                break;
            }
        }

        if (test_passed) {
            std::cout << "Test " << test_index << " passed." << std::endl;
        } else {
            std::cout << "Test " << test_index << " failed." << std::endl;
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
    bool run_tests = false;
    bool deep_test = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--test") {
            run_tests = true;
        } else if (arg == "--deep-test") {
            run_tests = true;
            deep_test = true;
        }
    }
    if (run_tests) {
        runTests(deep_test);
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
