#include "Engine.h"
#include "MoveGenerator.h"
#include "Evaluation.h"
#include "Lib.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <limits>
#include <climits>
#include <thread>
#include <vector>

const int SEARCH_DEPTH = 8;
const int ABORT_SCORE = std::numeric_limits<int>::max();
const int CHECKMATE_SCORE = 100000;

Engine::Engine(Comm *comm)
{
        this->comm = comm;
        this->comm->registerEngineGoCallback(
                [this](GoParams params) {
                        this->enqueueTask([this, params] { this->go(params); });
                });
        this->comm->registerEngineStopCallback([this] { this->stop(); });
        this->comm->registerEngineSetPositionCallback(
                [this](std::string position) {
                        this->enqueueTask([this, position] { this->setPosition(position); });
                });
        this->comm->registerEngineExecuteMoveCallback(
                [this](std::string move) {
                        this->enqueueTask([this, move] { this->executeMove(move); });
                });
        this->comm->registerEngineDebugCallback(
                [this] { this->enqueueTask([this] { this->debug(); }); });
        this->comm->registerEngineListMovesCallback(
                [this] { this->enqueueTask([this] { this->listMoves(); }); });
        this->comm->registerEnginePerftCallback(
                [this](int depth) {
                        this->enqueueTask([this, depth] { this->perft(depth); });
                });
        this->comm->registerEnginePerftDivideCallback(
                [this](int depth) {
                        this->enqueueTask([this, depth] { this->perftDivide(depth); });
                });
        this->comm->registerEnginePerftNodesCallback(
                [this](int depth) {
                        this->enqueueTask([this, depth] { this->perftNodes(depth); });
                        return 0ULL;
                });
	
        this->board = std::unique_ptr<Board>(new Board());
        this->perft_runner = std::unique_ptr<Perft>(new Perft());
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
        for (;;)
        {
                std::function<void()> task;
                {
                        std::unique_lock<std::mutex> lock(this->taskMutex);
                        this->taskCv.wait(lock, [this] { return !this->tasks.empty(); });
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                }
                task();
        }
}

void Engine::enqueueTask(std::function<void()> task)
{
        {
                std::lock_guard<std::mutex> lock(this->taskMutex);
                this->tasks.push(std::move(task));
        }
        this->taskCv.notify_one();
}

void Engine::stop()
{
        this->stopRequested = true;
}

void Engine::go(GoParams params)
{
        std::cout << "info string [Engine::go] here we go" << std::endl;

        int movetime = params.movetime;
        if (movetime <= 0)
        {
                char player = this->board->getPlayerToMove();
                int time = (player == 'w') ? params.wtime : params.btime;
                int inc = (player == 'w') ? params.winc : params.binc;
                if (time > 0)
                {
                        movetime = time / 40 + inc;
                        if (movetime <= 0)
                        {
                                movetime = time / 40;
                        }
                }
                if (movetime <= 0)
                {
                        movetime = 1000;
                }
        }

        std::cout << "info string [Engine::go] calculated movetime " << movetime << "ms" << std::endl;

        MoveGenerator moveGenerator;
        std::vector<std::string> possibleMoves = moveGenerator.getAllMoves(*this->board);
        std::cout << "info string [Engine::go] " << possibleMoves.size() << " moves found" << std::endl;

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
        auto timeExceeded = [&]() {
                return duration_cast<milliseconds>(steady_clock::now() - start).count() >= movetime;
        };

        bool rootMaximizing = this->board->getPlayerToMove() == 'w';
        std::string bestMove = possibleMoves[0];
        int bestScore = rootMaximizing ? std::numeric_limits<int>::min()
                                       : std::numeric_limits<int>::max();
        // `bestMoves` keeps all root moves that tie for the best score.
        // It is used as a fallback in case we abort before a principal
        // variation could be established.
        std::vector<std::string> bestMoves;
        // `bestPV` holds the principal variation returned by the search.
        // The first move in this line is the move reported as bestmove.
        std::vector<std::string> bestPV;
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
                // Per-depth containers storing all moves with the best
                // score and the principal variation for the best move at
                // the current depth.
                std::vector<std::string> depthBestMoves;
                std::vector<std::string> depthBestPV;

                int alpha = INT_MIN;
                int beta = INT_MAX;

                for (const auto& move : possibleMoves)
                {
                        Board nextBoard(*this->board);
                        nextBoard.executeMove(move);
                        // currentDepth counts plies including the move just played.
                        // After making a candidate move we have already spent one ply,
                        // therefore we search one ply less for the remaining moves.
                        SearchResult result = this->minimax(nextBoard,
                                                            currentDepth - 1,
                                                            alpha,
                                                            beta,
                                                            true,
                                                            timeExceeded);
                        if (result.score == ABORT_SCORE)
                        {
                                endReason = this->stopRequested ? "stop" : "movetime";
                                aborted = true;
                                break;
                        }
                        std::vector<std::string> line;
                        line.push_back(move);
                        line.insert(line.end(), result.moves.begin(), result.moves.end());
                        int score = result.score;
                        bool better = rootMaximizing ? (score > depthBestScore) : (score < depthBestScore);
                        if (better)
                        {
                                depthBestScore = score;
                                depthBestMove = move;
                                depthBestMoves.clear();
                                depthBestMoves.push_back(move);
                                depthBestPV = line;
                                auto now = steady_clock::now();
                                unsigned long long elapsed = duration_cast<milliseconds>(now - start).count();
                                unsigned long long nps = elapsed ? (this->nodes * 1000) / elapsed : 0;
                                this->emitInfo(elapsed, this->nodes, nps,
                                               depthBestScore, depthBestPV);
                        }
                        else if (score == depthBestScore)
                        {
                                depthBestMoves.push_back(move);
                        }

                        if (rootMaximizing)
                        {
                                if (score > alpha) alpha = score;
                        }
                        else
                        {
                                if (score < beta) beta = score;
                        }

                        auto now = steady_clock::now();
                        if (duration_cast<milliseconds>(now - lastInfo).count() >= 1000)
                        {
                                unsigned long long elapsed = duration_cast<milliseconds>(now - start).count();
                                unsigned long long nps = elapsed ? (this->nodes * 1000) / elapsed : 0;
                                this->emitInfo(elapsed, this->nodes, nps,
                                               depthBestScore, depthBestPV);

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
                        // Save the results of the fully searched depth so
                        // that we can fall back to them if a later depth is
                        // aborted.
                        bestMove = depthBestMove;
                        bestScore = depthBestScore;
                        bestMoves = depthBestMoves;
                        bestPV = depthBestPV;
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

        // Prefer the first move from the principal variation as it reflects
        // the search path we consider strongest. If no PV is available (for
        // example due to an early abort) fall back to the list of equally
        // good root moves. If both are empty, no legal move was found.
        if (!bestPV.empty())
        {
                bestMove = bestPV.front();
        }
        else if (!bestMoves.empty())
        {
                bestMove = bestMoves.front();
        }
        else
        {
                bestScore = 0;
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

        this->emitInfo(elapsed, this->nodes, nps, bestScore, bestPV);

        std::ostringstream output;
        output << "info string [Engine::go] best";
        for (const auto& move : bestPV)
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

SearchResult Engine::minimax(Board& board,
                             int depth,
                             int alpha,
                             int beta,
                             bool allowNull,
                             const std::function<bool()>& timeExceeded)
{
        if (this->stopRequested)
        {
                return {ABORT_SCORE, {}};
        }
        if (timeExceeded())
        {
                return {ABORT_SCORE, {}};
        }
        this->nodes++;

        MoveGenerator moveGenerator;
        std::vector<std::string> moves = moveGenerator.getAllMoves(board);
        bool maximizingPlayer = board.getPlayerToMove() == 'w';

        unsigned long long king_bb = maximizingPlayer ? (board.kings & board.whites)
                                                       : (board.kings & board.blacks);
        if (king_bb == 0)
        {
                return {maximizingPlayer ? -CHECKMATE_SCORE : CHECKMATE_SCORE, {}};
        }
        unsigned long long opponent_king_bb = maximizingPlayer ? (board.kings & board.blacks)
                                                               : (board.kings & board.whites);
        if (opponent_king_bb == 0)
        {
                return {maximizingPlayer ? CHECKMATE_SCORE : -CHECKMATE_SCORE, {}};
        }
        char opponent = maximizingPlayer ? 'b' : 'w';
        int king_sq = -1;
        bool inCheck = false;
        if (king_bb)
        {
                king_sq = __builtin_ffsll(king_bb) - 1;
                inCheck = moveGenerator.isSquareAttacked(board, king_sq, opponent);
        }

        if (depth == 0 || moves.empty())
        {
                if (moves.empty())
                {
                        if (inCheck)
                        {
                                return {maximizingPlayer ? -CHECKMATE_SCORE : CHECKMATE_SCORE, {}};
                        }
                        return {0, {}};
                }
                return {Evaluation::evaluate(board), {}};
        }

        // Null move pruning
        if (allowNull && depth >= 3 && !inCheck)
        {
                unsigned long long sidePieces = maximizingPlayer ? board.whites : board.blacks;
                unsigned long long nonPawnPieces = (board.queens | board.rooks | board.bishops | board.knights) & sidePieces;
                if (nonPawnPieces)
                {
                        Board nullBoard(board);
                        nullBoard.playerToMove = opponent;
                        nullBoard.enPassant = "-";
                        int R = 2;
                        if (maximizingPlayer)
                        {
                                SearchResult nullRes = minimax(nullBoard, depth - R - 1, beta - 1, beta, false, timeExceeded);
                                if (this->stopRequested || nullRes.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                                // Return the bound instead of the null move score to avoid
                                // propagating unrealistically high values.
                                if (nullRes.score >= beta) return {beta, {}};
                        }
                        else
                        {
                                SearchResult nullRes = minimax(nullBoard, depth - R - 1, alpha, alpha + 1, false, timeExceeded);
                                if (this->stopRequested || nullRes.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                                // Return the bound instead of the null move score to avoid
                                // propagating unrealistically low values.
                                if (nullRes.score <= alpha) return {alpha, {}};
                        }
                }
        }

        if (maximizingPlayer)
        {
                int maxEval = std::numeric_limits<int>::min();
                std::vector<std::string> bestLine;
                for (const auto& move : moves)
                {
                        Board nextBoard(board);
                        nextBoard.executeMove(move);
                        SearchResult result = minimax(nextBoard, depth - 1, alpha, beta, true, timeExceeded);
                        if (this->stopRequested || result.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                        if (result.score > maxEval)
                        {
                                maxEval = result.score;
                                bestLine = result.moves;
                                bestLine.insert(bestLine.begin(), move);
                        }
                        if (result.score > alpha) alpha = result.score;
                        if (beta <= alpha) break;
                }
                return {maxEval, bestLine};
        }
        else
        {
                int minEval = std::numeric_limits<int>::max();
                std::vector<std::string> bestLine;
                for (const auto& move : moves)
                {
                        Board nextBoard(board);
                        nextBoard.executeMove(move);
                        SearchResult result = minimax(nextBoard, depth - 1, alpha, beta, true, timeExceeded);
                        if (this->stopRequested || result.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                        if (result.score < minEval)
                        {
                                minEval = result.score;
                                bestLine = result.moves;
                                bestLine.insert(bestLine.begin(), move);
                        }
                        if (result.score < beta) beta = result.score;
                        if (beta <= alpha) break;
                }
                return {minEval, bestLine};
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
