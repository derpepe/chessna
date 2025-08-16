#include "Engine.h"
#include "MoveGenerator.h"
#include "Evaluation.h"
#include "Lib.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <limits>
#include <thread>
#include <vector>


Engine::Engine(Comm *comm)
{
	this->comm = comm;
        this->comm->registerEngineGoCallback( [this](int movetime) { this->go(movetime); } );
	this->comm->registerEngineSetPositionCallback( [this](std::string position) { this->setPosition(position); } );
        this->comm->registerEngineExecuteMoveCallback( [this](std::string move) { this->executeMove(move); } );
        this->comm->registerEngineDebugCallback( [this] { this->debug(); } );
        this->comm->registerEngineListMovesCallback( [this] { this->listMoves(); } );
        this->comm->registerEnginePerftCallback( [this](int depth) { this->perft(depth); } );
        this->comm->registerEnginePerftDivideCallback( [this](int depth) { this->perftDivide(depth); } );
        this->comm->registerEnginePerftNodesCallback( [this](int depth) { return this->perftNodes(depth); } );
	
	this->board = new Board();
	this->perft_runner = new Perft();
}

void Engine::setPosition(std::string position)
{
	std::cout << "info string [Engine:setPosition] settings position to '" << position << "'" << std::endl;
	this->board->loadFen(position);
}


void Engine::executeMove(std::string move)
{
	std::cout << "info string [Engine::executeMove] execute move '" << move << "'" << std::endl;
	this->board->executeMove(move);
}

void Engine::debug()
{
        this->comm->uciOutput(this->board->getDump());
}

void Engine::listMoves()
{
        MoveGenerator moveGenerator;
        std::vector<std::string> moves = moveGenerator.getAllMoves(*this->board);
        std::ostringstream output;
        output << "info string [Engine::listMoves]";
        for (const auto& move : moves)
        {
                output << ' ' << move;
        }
        output << std::endl;
        this->comm->uciOutput(output.str());
}

void Engine::run()
{
	while(true)
	{
		// main computation should go here
	}
}

void Engine::go(int movetime)
{
        std::cout << "info string [Engine::go] let's go!" << std::endl;

        MoveGenerator moveGenerator;
        std::vector<std::string> possibleMoves = moveGenerator.getAllMoves(*this->board);
        std::cout << "info string [Engine::go] found " << possibleMoves.size() << " moves" << std::endl;

        if (possibleMoves.empty())
        {
                std::cout << "info string [Engine::go] no moves found" << std::endl;
                return;
        }

        using namespace std::chrono;
        auto start = steady_clock::now();
        auto lastInfo = start;

        std::string bestMove = possibleMoves[0];
        int bestScore = std::numeric_limits<int>::min();
        std::vector<std::string> bestMoves;

        for (const auto& move : possibleMoves)
        {
                Board nextBoard(*this->board);
                nextBoard.executeMove(move);
                int score = this->minimax(nextBoard, 3, false);
                if (score > bestScore)
                {
                        bestScore = score;
                        bestMove = move;
                        bestMoves.clear();
                        bestMoves.push_back(move);
                }
                else if (score == bestScore)
                {
                        bestMoves.push_back(move);
                }

                auto now = steady_clock::now();
                if (duration_cast<milliseconds>(now - lastInfo).count() >= 1000)
                {
                        std::ostringstream status;
                        status << "info string [Engine::go] time "
                               << duration_cast<milliseconds>(now - start).count()
                               << "ms best " << bestMove << " score " << bestScore << std::endl;
                        this->comm->uciOutput(status.str());
                        lastInfo = now;
                }

                if (duration_cast<milliseconds>(now - start).count() >= movetime)
                {
                        break;
                }
        }

        // wait until allotted time has passed, emitting status each second
        auto endTime = start + std::chrono::milliseconds(movetime);
        while (std::chrono::steady_clock::now() < endTime)
        {
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInfo).count() >= 1000)
                {
                        std::ostringstream status;
                        status << "info string [Engine::go] time "
                               << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
                               << "ms best " << bestMove << " score " << bestScore << std::endl;
                        this->comm->uciOutput(status.str());
                        lastInfo = now;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // final status update at end of allotted time
        auto now = std::chrono::steady_clock::now();
        std::ostringstream status;
        status << "info string [Engine::go] time "
               << std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count()
               << "ms best " << bestMove << " score " << bestScore << std::endl;
        this->comm->uciOutput(status.str());

        std::ostringstream output;
        output << "info score cp " << bestScore << std::endl;
        output << "info string [Engine::go] equal";
        for (const auto& move : bestMoves)
        {
                output << ' ' << move;
        }
        output << std::endl;
        output << "bestmove " << bestMove << std::endl;
        this->comm->uciOutput(output.str());
}

int Engine::minimax(Board& board, int depth, bool maximizingPlayer)
{
        MoveGenerator moveGenerator;
        std::vector<std::string> moves = moveGenerator.getAllMoves(board);

        if (depth == 0 || moves.empty())
        {
                if (moves.empty()) return 0;
                int bestScore = maximizingPlayer ? -100000 : 100000;
                for (const auto& move : moves)
                {
                        int score = Evaluation::evaluate(board, move);
                        if (maximizingPlayer)
                        {
                                if (score > bestScore) bestScore = score;
                        }
                        else
                        {
                                if (score < bestScore) bestScore = score;
                        }
                }
                return bestScore;
        }

        if (maximizingPlayer)
        {
                int maxEval = -100000;
                for (const auto& move : moves)
                {
                        Board nextBoard(board);
                        nextBoard.executeMove(move);
                        if (depth > 1)
                                std::cout << "info string [Engine::minimax] depth " << (depth - 1) << std::endl;
                        int eval = minimax(nextBoard, depth - 1, false);
                        if (eval > maxEval) maxEval = eval;
                }
                return maxEval;
        }
        else
        {
                int minEval = 100000;
                for (const auto& move : moves)
                {
                        Board nextBoard(board);
                        nextBoard.executeMove(move);
                        if (depth > 1)
                                std::cout << "info string [Engine::minimax] depth " << (depth - 1) << std::endl;
                        int eval = minimax(nextBoard, depth - 1, true);
                        if (eval < minEval) minEval = eval;
                }
                return minEval;
        }
}

void Engine::perft(int depth)
{
	std::cout << "info string [Engine::perft] starting perft(" << depth << ")" << std::endl;
	PerftResult result = this->perft_runner->perft(*this->board, depth);
        std::ostringstream output;
        output << "info string [Engine::perft] nodes " << Lib::formatThousands(result.nodes) << std::endl
                << "info string [Engine::perft] captures " << Lib::formatThousands(result.captures) << std::endl
                << "info string [Engine::perft] ep " << Lib::formatThousands(result.en_passant) << std::endl
                << "info string [Engine::perft] castles " << Lib::formatThousands(result.castles) << std::endl
                << "info string [Engine::perft] promotions " << Lib::formatThousands(result.promotions) << std::endl
                << "info string [Engine::perft] checks " << Lib::formatThousands(result.checks) << std::endl
                << "info string [Engine::perft] checkmates " << Lib::formatThousands(result.checkmates) << std::endl;
	this->comm->uciOutput(output.str());
}

void Engine::perftDivide(int depth)
{
        std::cout << "info string [Engine::perftDivide] starting perftDivide(" << depth << ")" << std::endl;
        this->perft_runner->perftDivide(*this->board, depth);
}

unsigned long long Engine::perftNodes(int depth)
{
        PerftResult result = this->perft_runner->perft(*this->board, depth);
        std::ostringstream output;
        output << "info string [Engine::perftNodes] perft(" << depth
               << ") nodes " << Lib::formatThousands(result.nodes) << std::endl;
        this->comm->uciOutput(output.str());
        return result.nodes;
}
