#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Comm.h"
#include "Perft.h"
#include <vector>
#include <string>
#include <random>
#include <atomic>

class Engine
{
public:
	Engine(Comm* comm);	
	void run();
	
	// commands sent via UCI
        void go(int);
        void setPosition(std::string);
        void executeMove(std::string);
        void debug();
        void listMoves();
        void perft(int);
        void perftDivide(int);
        unsigned long long perftNodes(int);

        void stop();

private:
        Board *board;
        Comm *comm;
        Perft *perft_runner;

        std::atomic<bool> stopRequested;
        unsigned long long nodes;

        int minimax(Board& board, int depth);

        void emitInfo(unsigned long long elapsed,
                       unsigned long long nodes,
                       unsigned long long nps,
                       int score,
                       const std::vector<std::string>& pv);
};

#endif
