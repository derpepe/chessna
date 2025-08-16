#include "Tests.h"
#include "Board.h"
#include "Perft.h"
#include "Evaluation.h"
#include "perft_data.h"
#include <iostream>
#include <vector>
#include <sstream>

std::string Tests::extractFen(const std::string& dump) {
    auto pos = dump.find("FEN: ");
    if (pos == std::string::npos) return "";
    auto end = dump.find('\n', pos);
    std::string fen = dump.substr(pos + 5, end - (pos + 5));
    while (!fen.empty() && (fen.back() == '\r' || fen.back() == ' ')) {
        fen.pop_back();
    }
    return fen;
}

bool Tests::testFenLoading() {
    std::cout << "Running FEN loading tests" << std::endl;
    bool all = true;
    for (int i = 0; i < 5 && PERFT_DATA[i]; ++i) {
        std::string line(PERFT_DATA[i]);
        std::size_t pos = line.find(" -,");
        if (pos == std::string::npos) continue;
        std::string fen = line.substr(0, pos + 2) + " 0 1";
        Board b;
        b.loadFen(fen);
        std::string loaded = extractFen(b.getDump());
        bool ok = !loaded.empty();
        std::cout << "  Test " << i + 1 << (ok ? " passed" : " failed") << std::endl;
        if (!ok) all = false;
    }
    return all;
}

bool Tests::testPerft() {
    std::cout << "Running perft tests" << std::endl;
    bool all = true;
    struct PerftCase { std::string fen; std::vector<unsigned long long> nodes; };
    std::vector<PerftCase> tests;
    for (int i = 0; i < 5 && PERFT_DATA[i]; ++i) {
        std::string line(PERFT_DATA[i]);
        std::size_t pos = line.find(" -,");
        if (pos == std::string::npos) continue;
        PerftCase pc; pc.fen = line.substr(0, pos + 2) + " 0 1";
        std::string numbers = line.substr(pos + 3);
        std::stringstream ss(numbers); std::string num;
        while (std::getline(ss, num, ',')) {
            if (!num.empty()) pc.nodes.push_back(std::stoull(num));
        }
        tests.push_back(pc);
    }
    for (size_t i = 0; i < tests.size(); ++i) {
        Board b; b.loadFen(tests[i].fen); Perft p; bool ok = true;
        size_t depth = std::min<size_t>(3, tests[i].nodes.size());
        for (size_t d = 0; d < depth; ++d) {
            auto res = p.perft(b, d + 1);
            if (res.nodes != tests[i].nodes[d]) { ok = false; break; }
        }
        std::cout << "  Test " << i + 1 << (ok ? " passed" : " failed") << std::endl;
        if (!ok) all = false;
    }
    return all;
}

bool Tests::testEvaluation() {
    std::cout << "Running evaluation tests" << std::endl;
    bool all = true;
    struct EvalCase { std::string fen; int score; };
    std::vector<EvalCase> tests = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 0},
        {"rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 900},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1", -900},
        {"rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1", 22},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1", 20},
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        Board b; b.loadFen(tests[i].fen);
        int score = Evaluation::evaluate(b);
        bool ok = (score == tests[i].score);
        std::cout << "  Test " << i + 1 << (ok ? " passed" : " failed") << std::endl;
        if (!ok) all = false;
    }
    return all;
}

bool Tests::runAll() {
    bool ok = true;
    if (!testFenLoading()) ok = false;
    if (!testPerft()) ok = false;
    if (!testEvaluation()) ok = false;
    if (ok) std::cout << "All tests passed!" << std::endl;
    else std::cout << "Some tests failed." << std::endl;
    return ok;
}
