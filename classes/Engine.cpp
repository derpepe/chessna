#include "Engine.h"
#include "MoveGenerator.h"
#include "Evaluator.h"
#include "Lib.h"
#include <iostream>


Engine::Engine(Comm *comm)
{
	this->comm = comm;
	this->comm->registerEngineGoCallback( [this] { this->go(); } );
	this->comm->registerEngineSetPositionCallback( [this](std::string position) { this->setPosition(position); } );
	this->comm->registerEngineExecuteMoveCallback( [this](std::string move) { this->executeMove(move); } );
	this->comm->registerEngineDebugCallback( [this] { this->debug(); } );
	this->comm->registerEnginePerftCallback( [this](int depth) { this->perft(depth); } );
	this->comm->registerEnginePerftDivideCallback( [this](int depth) { this->perftDivide(depth); } );
	
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

void Engine::run()
{
	while(true)
	{
		// main computation should go here
	}
}

void Engine::go()
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

        std::vector<std::string> bestMoves;
        int bestScore = -100000;

        for (const auto& move : possibleMoves)
        {
                Board nextBoard(*this->board);
                nextBoard.executeMove(move);
                int score = this->minimax(nextBoard, 3, false);
                if (score > bestScore)
                {
                        bestScore = score;
                        bestMoves.clear();
                        bestMoves.push_back(move);
                }
                else if (score == bestScore)
                {
                        bestMoves.push_back(move);
                }
        }

        // Output all moves sharing the best score
        std::cout << "info string [Engine::go] best moves:";
        for (const auto& move : bestMoves)
        {
                std::cout << ' ' << move;
        }
        std::cout << std::endl;

        // Choose one of the best moves at random
        std::random_device seed;
        std::mt19937 engine(seed());
        std::uniform_int_distribution<size_t> choose(0, bestMoves.size() - 1);
        std::string bestMove = bestMoves[choose(engine)];

        std::ostringstream output;
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
                        int score = Evaluator::evaluate(board, move);
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
