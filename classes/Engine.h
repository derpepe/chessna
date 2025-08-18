#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <array>
#include <cstdint>

#include "Board.h"
#include "MoveGenerator.h"
#include "Evaluation.h"
#include "Perft.h"
#include "Comm.h"

struct SearchResult
{
        int score;
        std::vector<std::string> moves;
};

struct TTEntry
{
        std::uint64_t key = 0;
        int depth = -1;              // search depth of this entry
        int score = 0;               // score in centipawns (includes mate scores)
        enum {FLAG_EXACT=0, FLAG_ALPHA=1, FLAG_BETA=2} flag = FLAG_EXACT;
        std::string bestMove;        // best move as UCI string for ordering
};

class Engine
{
public:
        explicit Engine(Comm* comm);

        // UCI-Entry points
        void go(GoParams params);
        void stop() { stopRequested = true; }
		
        void setPosition(const std::string& fen, const std::vector<std::string>& moves)
        {
            // Board sauber zurücksetzen (Board ist ein Wert, kein Pointer)
            board = Board();
        
            // Arbeitskopie anlegen, weil fen const& ist
            std::string fenStr = fen;
            if (fenStr.empty() || fenStr == "startpos") {
                fenStr = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
            }
        
            // loadFen gibt void zurück → keine Zuweisung
            board.loadFen(fenStr);
        
            for (const auto& m : moves) {
                board.executeMove(m);
            }
        }

        // Perft utilities
        void perft(int depth);
        void perftDivide(int depth);
        unsigned long long perftNodes(int depth);

        // Info
        void emitInfo(unsigned long long elapsed,
                      unsigned long long nodes,
                      unsigned long long nps,
                      int score,
                      const std::vector<std::string>& pv);

        // Accessors
        char getPlayerToMove() const { return board.getPlayerToMove(); }

        // Search
        SearchResult minimax(Board& board,
                             int depth,
                             int alpha,
                             int beta,
                             const std::function<bool()>& timeExceeded,
                             int ply);

        int quiescence(Board& board,
                       int alpha,
                       int beta,
                       const std::function<bool()>& timeExceeded,
                       int ply);

        void applyMove(const std::string& m) { board.executeMove(m); }

        // Helpers
        std::uint64_t computeZobristKey(const Board&) const;
        bool is_capture_move(const Board& b, const std::string& move, bool whiteToMove) const;
        int  mvv_lva_score(const Board& b, const std::string& move, bool whiteToMove) const;

private:
        static constexpr int MAX_PLY = 128;

        Comm* comm = nullptr;
        Board board;
        Perft perft_runner;

        // Search state
        unsigned long long nodes = 0;
        bool stopRequested = false;

        // Move ordering helpers
        std::array<std::array<std::string,2>, MAX_PLY> killerMoves{}; // two killers per ply
        int history[2][64][64] = {};  // side (0=w, 1=b) x from x to

        // Transposition Table
        std::unordered_map<std::uint64_t, TTEntry> tt;
        std::size_t ttMaxEntries = 200'000; // small VM friendly
};

#endif
