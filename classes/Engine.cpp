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
#include <random>

const int SEARCH_DEPTH = 4;

Engine::Engine(Comm *comm)
{
        this->comm = comm;
        this->comm->registerEngineGoCallback( [this](int movetime) { this->go(movetime); } );
        this->comm->registerEngineStopCallback( [this] { this->stop(); } );
        this->comm->registerEngineSetPositionCallback( [this](std::string position) { this->setPosition(position); } );
        this->comm->registerEngineExecuteMoveCallback( [this](std::string move) { this->executeMove(move); } );
        this->comm->registerEngineDebugCallback( [this] { this->debug(); } );
        this->comm->registerEngineListMovesCallback( [this] { this->listMoves(); } );
        this->comm->registerEnginePerftCallback( [this](int depth) { this->perft(depth); } );
        this->comm->registerEnginePerftDivideCallback( [this](int depth) { this->perftDivide(depth); } );
        this->comm->registerEnginePerftNodesCallback( [this](int depth) { return this->perftNodes(depth); } );
	
        this->board = new Board();
        this->perft_runner = new Perft();
        this->stopRequested = false;
        this->nodes = 0;
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

void Engine::stop()
{
        this->stopRequested = true;
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

        this->stopRequested = false;
        this->nodes = 0;

        using namespace std::chrono;
        auto start = steady_clock::now();
        auto lastInfo = start;

        bool rootMaximizing = this->board->getPlayerToMove() == 'w';
        std::string bestMove = possibleMoves[0];
        int bestScore = rootMaximizing ? std::numeric_limits<int>::min()
                                       : std::numeric_limits<int>::max();
        std::vector<std::string> bestMoves;
        std::string endReason;
        bool aborted = false;

        for (int currentDepth = 1; currentDepth <= SEARCH_DEPTH; ++currentDepth)
        {
                std::ostringstream depthInfo;
                depthInfo << "info depth " << currentDepth << std::endl;
                this->comm->uciOutput(depthInfo.str());

                std::string depthBestMove = possibleMoves[0];
                int depthBestScore = rootMaximizing ? std::numeric_limits<int>::min()
                                                    : std::numeric_limits<int>::max();
                std::vector<std::string> depthBestMoves;

                for (const auto& move : possibleMoves)
                {
                        Board nextBoard(*this->board);
                        nextBoard.executeMove(move);
                        // currentDepth counts plies including the move just played.
                        // After making a candidate move we have already spent one ply,
                        // therefore we search one ply less for the remaining moves.
                        int score = this->minimax(nextBoard, currentDepth - 1);
                        bool better = rootMaximizing ? (score > depthBestScore) : (score < depthBestScore);
                        if (better)
                        {
                                depthBestScore = score;
                                depthBestMove = move;
                                depthBestMoves.clear();
                                depthBestMoves.push_back(move);
                        }
                        else if (score == depthBestScore)
                        {
                                depthBestMoves.push_back(move);
                        }

                        auto now = steady_clock::now();
                        if (duration_cast<milliseconds>(now - lastInfo).count() >= 1000)
                        {
                                unsigned long long elapsed = duration_cast<milliseconds>(now - start).count();
                                unsigned long long nps = elapsed ? (this->nodes * 1000) / elapsed : 0;
                                this->emitInfo(elapsed, this->nodes, nps,
                                               depthBestScore, depthBestMoves);

                                lastInfo = now;
                        }

                        if (this->stopRequested)
                        {
                                endReason = "stop";
                                aborted = true;
                                break;
                        }
                        if (duration_cast<milliseconds>(now - start).count() >= movetime)
                        {
                                endReason = "movetime";
                                aborted = true;
                                break;
                        }
                }

                if (!aborted)
                {
                        bestMove = depthBestMove;
                        bestScore = depthBestScore;
                        bestMoves = depthBestMoves;
                }
                else
                {
                        break;
                }
        }

        if (endReason.empty())
        {
                endReason = "moves";
        }

        if (!bestMoves.empty())
        {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, bestMoves.size() - 1);
                bestMove = bestMoves[dis(gen)];
        }

        auto now = std::chrono::steady_clock::now();
        std::ostringstream reason;
        reason << "info string [Engine::go] search finished: ";
        if (endReason == "movetime")
        {
                reason << "movetime";
        }
        else if (endReason == "stop")
        {
                reason << "stop command";
        }
        else
        {
                reason << "all moves searched";
        }
        reason << std::endl;
        this->comm->uciOutput(reason.str());

        unsigned long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        unsigned long long nps = elapsed ? (this->nodes * 1000) / elapsed : 0;

        this->emitInfo(elapsed, this->nodes, nps, bestScore, bestMoves);

        std::ostringstream output;
        output << "info string [Engine::go] best";
        for (const auto& move : bestMoves)
        {
                output << ' ' << move;
        }
        output << std::endl;
        output << "bestmove " << bestMove << std::endl;
        this->comm->uciOutput(output.str());
}

void Engine::emitInfo(unsigned long long elapsed,
                      unsigned long long nodes,
                      unsigned long long nps,
                      int score,
                      const std::vector<std::string>& pv)
{
        std::ostringstream status;
        status << "info string [Engine::go] time "
               << elapsed
               << "ms nodes " << Lib::formatThousands(nodes)
               << " nps " << Lib::formatThousands(nps)
               << " best";
        for (const auto& mv : pv)
        {
                status << ' ' << mv;
        }
        status << " score " << score << std::endl;
        this->comm->uciOutput(status.str());

        std::ostringstream uci;
        uci << "info time " << elapsed
            << " nodes " << nodes
            << " nps " << nps
            << " score cp " << score << " pv";
        for (const auto& mv : pv)
        {
                uci << ' ' << mv;
        }
        uci << std::endl;
        this->comm->uciOutput(uci.str());
}

int Engine::minimax(Board& board, int depth)
{
        this->nodes++;
        MoveGenerator moveGenerator;
        std::vector<std::string> moves = moveGenerator.getAllMoves(board);

        if (depth == 0 || moves.empty())
        {
                if (moves.empty()) return 0;
                return Evaluation::evaluate(board);
        }

        bool maximizingPlayer = board.getPlayerToMove() == 'w';

        if (maximizingPlayer)
        {
                int maxEval = -100000;
                for (const auto& move : moves)
                {
                        Board nextBoard(board);
                        nextBoard.executeMove(move);
                        int eval = minimax(nextBoard, depth - 1);
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
                        int eval = minimax(nextBoard, depth - 1);
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
