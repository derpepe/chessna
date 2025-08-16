#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Comm.h"
#include "Perft.h"
#include <vector>
#include <string>
#include <random>

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

private:
        Board *board;
        Comm *comm;
        Perft *perft_runner;

        int minimax(Board& board, int depth);
};

#endif
