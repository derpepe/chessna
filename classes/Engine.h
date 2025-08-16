#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Comm.h"
#include "Perft.h"
#include <vector>
#include <string>
#include <random>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>

struct SearchResult
{
        int score;
        std::vector<std::string> moves;
};

class Engine
{
public:
	Engine(Comm* comm);	
	void run();
	
        // commands sent via UCI
        void go(GoParams);
        void setPosition(std::string);
        void executeMove(std::string);
        void debug();
        void listMoves();
        void perft(int);
        void perftDivide(int);
        unsigned long long perftNodes(int);

        void stop();

private:
        std::unique_ptr<Board> board;
        Comm *comm;
        std::unique_ptr<Perft> perft_runner;

        std::atomic<bool> stopRequested;
        unsigned long long nodes;

        std::mutex taskMutex;
        std::condition_variable taskCv;
        std::queue<std::function<void()>> tasks;

        SearchResult minimax(Board& board,
                             int depth,
                             int alpha,
                             int beta,
                             bool allowNull,
                             const std::function<bool()>& timeExceeded);

        void emitInfo(unsigned long long elapsed,
                       unsigned long long nodes,
                       unsigned long long nps,
                       int score,
                       const std::vector<std::string>& pv);

        void enqueueTask(std::function<void()> task);
};

#endif
