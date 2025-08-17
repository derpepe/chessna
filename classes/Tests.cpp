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
    struct EvalCase { std::string fen; int minScore; int maxScore; };
    std::vector<EvalCase> tests = {
        // Starting position should be roughly equal
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", -50, 50},
        // Black missing queen -> large advantage for white
        {"rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 800, 2000},
        // White missing queen -> large advantage for black
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1", -2000, -800},
        // Advanced passed pawn on 7th rank
        {"7k/P7/8/8/8/8/8/7K w - - 0 1", 50, 150},
        // A lone rook against a king should reflect the material advantage
        // for White in this simplified evaluation function.
        {"6k1/6R1/8/8/8/8/8/7K w - - 0 1", 400, 600},
        // Pawn in the center should give a small bonus to White
        {"rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1", 5, 15},
        // Immediate checkmate against the side to move
        {"7k/7Q/7K/8/8/8/8/8 b - - 0 1", 900000, 1000000}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        Board b; b.loadFen(tests[i].fen);
        int score = Evaluation::evaluate(b);
        bool ok = (score >= tests[i].minScore && score <= tests[i].maxScore);
        std::cout << "  Test " << i + 1 << (ok ? " passed" : " failed") << std::endl;
        if (!ok) all = false;
    }
    return all;
}

bool Tests::testExecuteMove() {
    std::cout << "Running executeMove tests" << std::endl;
    struct ExecCase { std::vector<std::string> moves; std::string fen; };
    const std::string startFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    std::vector<ExecCase> tests = {
        { {"e2e4"},
          "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1" },
        { {"e2e4", "d7d5", "e4d5"},
          "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2" },
        { {"g1f3", "b8c6", "e2e4", "e7e5", "f1c4", "g8f6", "e1g1"},
          "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQ1RK1 b kq - 3 4" },
        { {"e2e4", "d7d5", "e4e5", "f7f5", "e5f6"},
          "rnbqkbnr/ppp1p1pp/5P2/3p4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 3" },
        { {"g2g4", "e7e5", "e2e4", "g8f6", "d1f3", "b8c6", "f1h3", "f8b4", "c2c3", "b4c5", "d2d4", "e5d4", "f3f5", "d4c3", "f5c5", "d7d6", "c1g5", "e8g8"},
          "r1bq1rk1/ppp2ppp/2np1n2/2Q3B1/4P1P1/2p4B/PP3P1P/RN2K1NR w KQ - 2 10" }
    };

    bool all = true;
    for (size_t i = 0; i < tests.size(); ++i) {
        Board b;
        b.loadFen(startFen);
        for (const auto& mv : tests[i].moves) {
            b.executeMove(mv);
        }
        std::string fen = extractFen(b.getDump());
        bool ok = (fen == tests[i].fen);
        std::cout << "  Test " << i + 1 << (ok ? " passed" : " failed") << std::endl;
        if (!ok) {
            std::cout << "    expected: " << tests[i].fen << std::endl;
            std::cout << "    got     : " << fen << std::endl;
            all = false;
        }
    }
    return all;
}

bool Tests::runAll() {
    bool ok = true;
    if (!testFenLoading()) ok = false;
    if (!testPerft()) ok = false;
    if (!testEvaluation()) ok = false;
    if (!testExecuteMove()) ok = false;
    if (ok) std::cout << "All tests passed!" << std::endl;
    else std::cout << "Some tests failed." << std::endl;
    return ok;
}
