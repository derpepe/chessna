#include <thread>
#include <mutex>
#include <atomic>
#include "Uci.h"
#include "Engine.h"
#include "Comm.h"
#include "Board.h"
#include "Perft.h"
#include "Tests.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <memory>
#include "perft_data.h"

void runPerftTests(bool deepTest, int singleTest = 0) {
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

    if (singleTest > 0 && static_cast<size_t>(singleTest) > tests.size()) {
        std::cout << "Test " << singleTest << " not found." << std::endl;
        return;
    }

    // Run only the requested test if a specific number is provided
    if (singleTest > 0) {
        const auto& test = tests[singleTest - 1];
        std::cout << "Running test " << singleTest << ": " << test.fen << std::endl;

        Board board;
        board.loadFen(test.fen);
        Perft perft;

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
                test_passed = false;
                break;
            }
        }

        std::cout << "Test " << singleTest << (test_passed ? " passed." : " failed.") << std::endl;
        return;
    }

    std::atomic<bool> all_tests_passed{true};
    std::atomic<std::size_t> next_index{0};
    std::mutex cout_mutex;

    unsigned int thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) {
        thread_count = 2;
    }

    auto worker = [&]() {
        Perft perft;
        std::ostringstream out;
        while (true) {
            std::size_t idx = next_index++;
            if (idx >= tests.size()) {
                break;
            }

            const auto& test = tests[idx];
            Board board;
            board.loadFen(test.fen);

            bool test_passed = true;
            out.str("");
            out.clear();
            out << "Running test " << idx + 1 << ": " << test.fen << std::endl;

            std::size_t max_depth = deepTest ? test.nodes.size() : std::min<std::size_t>(4, test.nodes.size());
            for (std::size_t i = 0; i < max_depth; ++i) {
                int depth = static_cast<int>(i + 1);
                PerftResult result = perft.perft(board, depth);
                bool depth_passed = (result.nodes == test.nodes[i]);

                out << "  Depth " << depth << ": expected " << test.nodes[i]
                    << ", got " << result.nodes
                    << (depth_passed ? " [OK]" : " [FAIL]") << std::endl;

                if (!depth_passed) {
                    all_tests_passed = false;
                    test_passed = false;
                    break;
                }
            }

            out << "Test " << idx + 1 << (test_passed ? " passed." : " failed.") << std::endl;

            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << out.str();
            }
        }
    };

    std::vector<std::thread> threads;
    for (unsigned int i = 0; i < thread_count; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) {
        t.join();
    }

    if (all_tests_passed) {
        std::cout << "All perft tests passed!" << std::endl;
    } else {
        std::cout << "Some perft tests failed." << std::endl;
    }
}

int main(int argc, char** argv)
{
    bool run_perft = false;
    bool full_perft = false;
    bool run_suite = false;
    int test_number = 0;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--perft") {
            run_perft = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                test_number = std::stoi(argv[++i]);
            }
        } else if (arg == "--full-perft") {
            run_perft = true;
            full_perft = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                test_number = std::stoi(argv[++i]);
            }
        } else if (arg == "--test") {
            run_suite = true;
        }
    }
    if (run_perft) {
        runPerftTests(full_perft, test_number);
        return 0;
    }
    if (run_suite) {
        Tests t;
        t.runAll();
        return 0;
    }

        auto comm = std::unique_ptr<Comm>(new Comm());

        auto uci = std::unique_ptr<Uci>(new Uci(comm.get()));
        auto engine = std::unique_ptr<Engine>(new Engine(comm.get()));

        uci->run();
        return 0;
}
