#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Comm.h"
#include "Perft.h"
#include <vector>
#include <string>
#include <functional>

struct SearchResult
{
        int score;
        std::vector<std::string> moves;
};

class Engine
{
public:
        Engine(Comm* comm);
	
        // commands sent via UCI
        void go(GoParams);
        void setPosition(std::string);
        void executeMove(std::string);
        void debug();
        void listMoves();
        void evaluate();
        void perft(int);
        void perftDivide(int);
        unsigned long long perftNodes(int);

        void stop();

private:
        Board board;
        Comm *comm;
        Perft perft_runner;

        bool stopRequested;
        unsigned long long nodes;

        SearchResult minimax(Board& board,
                             int depth,
                             int alpha,
                             int beta,
                             const std::function<bool()>& timeExceeded,
                             int ply);

        void emitInfo(unsigned long long elapsed,
                       unsigned long long nodes,
                       unsigned long long nps,
                       int score,
                       const std::vector<std::string>& pv);
};

#endif
