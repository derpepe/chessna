// Engine.cpp
#include "Engine.h"
#include "Evaluation.h"
#include "MoveGenerator.h"
#include "Lib.h"

#include <limits>
#include <algorithm>
#include <random>
#include <array>
#include <unordered_map>
#include <cstdint>
#include <chrono>
#include <sstream>
#include <iostream>

static const int ABORT_SCORE     = std::numeric_limits<int>::max();
static const int CHECKMATE_SCORE = 100000;

// ---------------- Zobrist Hashing (TT) ----------------
namespace {
        bool zobristInitialized = false;
        std::array<std::array<std::uint64_t,64>, 12> ZOBRIST_PIECE{}; // 12 piece types: WP,WN,WB,WR,WQ,WK, BP,BN,BB,BR,BQ,BK
        std::array<std::uint64_t, 16> ZOBRIST_CASTLING{};             // 4 castling rights -> 16 combos
        std::array<std::uint64_t, 8>  ZOBRIST_ENPASSANT_FILE{};       // en-passant file a..h
        std::uint64_t ZOBRIST_SIDE = 0;

        inline void init_zobrist()
        {
                if (zobristInitialized) return;
                std::mt19937_64 rng(0xABCDEF1234567890ULL); // fixed seed for reproducibility
                auto rnd = [&](){ return rng(); };

                for (auto &arr : ZOBRIST_PIECE)
                        for (auto &x : arr) x = rnd();

                for (auto &x : ZOBRIST_CASTLING) x = rnd();
                for (auto &x : ZOBRIST_ENPASSANT_FILE) x = rnd();
                ZOBRIST_SIDE = rnd();

                zobristInitialized = true;
        }
}

// ---------------- Engine ----------------
Engine::Engine(Comm *comm)
{
        this->comm = comm;
        this->nodes = 0;
        this->stopRequested = false;
}

// Zobrist key
std::uint64_t Engine::computeZobristKey(const Board& b) const
{
        init_zobrist();
        std::uint64_t key = 0;

        auto addPieces = [&](bool white){
                unsigned long long side = white ? b.whites : b.blacks;

                auto add = [&](unsigned long long bb, int idxWhite, int idxBlack){
                        unsigned long long set = bb & side;
                        while (set)
                        {
                                int sq = __builtin_ffsll(set) - 1;
                                int pidx = white ? idxWhite : idxBlack;
                                key ^= ZOBRIST_PIECE[pidx][sq];
                                set &= (set - 1);
                        }
                };
                add(b.pawns,   0,  6);
                add(b.knights, 1,  7);
                add(b.bishops, 2,  8);
                add(b.rooks,   3,  9);
                add(b.queens,  4, 10);
                add(b.kings,   5, 11);
        };

        addPieces(true);
        addPieces(false);

        int cr = 0;
        if (b.casteling_K) cr |= 1;
        if (b.casteling_Q) cr |= 2;
        if (b.casteling_k) cr |= 4;
        if (b.casteling_q) cr |= 8;
        key ^= ZOBRIST_CASTLING[cr];

        if (!b.enPassant.empty() && b.enPassant != "-")
        {
                int sq = Lib::getBitnumFromCoordinates(b.enPassant);
                int file = Lib::getFile(sq);
                key ^= ZOBRIST_ENPASSANT_FILE[file];
        }

        if (b.playerToMove == 'w') key ^= ZOBRIST_SIDE;

        return key;
}

// capture & MVV-LVA
bool Engine::is_capture_move(const Board& b, const std::string& move, bool whiteToMove) const
{
        if (move.size() < 4) return false;
        int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
        int to   = Lib::getBitnumFromCoordinates(move.substr(2,2));

        unsigned long long opp = whiteToMove ? b.blacks : b.whites;
        if ((opp >> to) & 1ULL) return true;

        if (!b.enPassant.empty() && b.enPassant != "-")
        {
            int epsq = Lib::getBitnumFromCoordinates(b.enPassant);
            if (to == epsq)
            {
                unsigned long long myPawns = b.pawns & (whiteToMove ? b.whites : b.blacks);
                if ((myPawns >> from) & 1ULL)
                {
                    int fromFile = Lib::getFile(from);
                    int toFile = Lib::getFile(to);
                    if (fromFile != toFile) return true;
                }
            }
        }
        return false;
}

int Engine::mvv_lva_score(const Board& b, const std::string& move, bool whiteToMove) const
{
        auto val = [&](char p){
                switch(p){
                        case 'P': case 'p': return 100;
                        case 'N': case 'n': return 320;
                        case 'B': case 'b': return 330;
                        case 'R': case 'r': return 500;
                        case 'Q': case 'q': return 900;
                        case 'K': case 'k': return 20000;
                        default: return 0;
                }
        };

        if (move.size() < 4) return 0;
        int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
        int to   = Lib::getBitnumFromCoordinates(move.substr(2,2));

        char attacker = 0;
        unsigned long long my = whiteToMove ? b.whites : b.blacks;
        if ((b.pawns   & my) & (1ULL << from)) attacker = whiteToMove ? 'P' : 'p';
        else if ((b.knights & my) & (1ULL << from)) attacker = whiteToMove ? 'N' : 'n';
        else if ((b.bishops & my) & (1ULL << from)) attacker = whiteToMove ? 'B' : 'b';
        else if ((b.rooks   & my) & (1ULL << from)) attacker = whiteToMove ? 'R' : 'r';
        else if ((b.queens  & my) & (1ULL << from)) attacker = whiteToMove ? 'Q' : 'q';
        else if ((b.kings   & my) & (1ULL << from)) attacker = whiteToMove ? 'K' : 'k';

        char victim = 0;
        unsigned long long opp = whiteToMove ? b.blacks : b.whites;
        if ((b.pawns   & opp) & (1ULL << to)) victim = whiteToMove ? 'p' : 'P';
        else if ((b.knights & opp) & (1ULL << to)) victim = whiteToMove ? 'n' : 'N';
        else if ((b.bishops & opp) & (1ULL << to)) victim = whiteToMove ? 'b' : 'B';
        else if ((b.rooks   & opp) & (1ULL << to)) victim = whiteToMove ? 'r' : 'R';
        else if ((b.queens  & opp) & (1ULL << to)) victim = whiteToMove ? 'q' : 'Q';
        else if (!b.enPassant.empty() && b.enPassant != "-") {
            int epsq = Lib::getBitnumFromCoordinates(b.enPassant);
            if (to == epsq) victim = whiteToMove ? 'p' : 'P';
        }

        int score = 0;
        if (victim) score += 10000 + 10 * val(victim) - val(attacker);
        if (move.size() >= 5) score += 900;
        return score;
}

// --------------- Quiescence ---------------
int Engine::quiescence(Board& board,
                       int alpha,
                       int beta,
                       const std::function<bool()>& timeExceeded,
                       int ply)
{
        if (this->stopRequested || timeExceeded())
                return ABORT_SCORE;

        this->nodes++;

        int stand_pat = Evaluation::evaluate(board);
        if (stand_pat >= beta) return beta;
        if (stand_pat > alpha) alpha = stand_pat;

        MoveGenerator gen;
        std::vector<std::string> moves = gen.getAllMoves(board);
        bool whiteToMove = board.getPlayerToMove() == 'w';

        std::vector<std::pair<int,std::string>> captures;
        captures.reserve(moves.size());
        for (const auto& m : moves)
        {
                if (is_capture_move(board, m, whiteToMove))
                {
                        int s = mvv_lva_score(board, m, whiteToMove);
                        captures.emplace_back(s, m);
                }
        }
        std::sort(captures.begin(), captures.end(),
                  [](const auto& a, const auto& b){ return a.first > b.first; });

        for (const auto& sm : captures)
        {
                const std::string& move = sm.second;
                Board next(board);
                next.executeMove(move);
                int score = quiescence(next, -beta, -alpha, timeExceeded, ply+1);
                if (score == ABORT_SCORE) return ABORT_SCORE;
                score = -score;

                if (score >= beta) return beta;
                if (score > alpha) alpha = score;
        }
        return alpha;
}

// --------------- Minimax ---------------
SearchResult Engine::minimax(Board& board,
                             int depth,
                             int alpha,
                             int beta,
                             const std::function<bool()>& timeExceeded,
                             int ply)
{
        if (this->stopRequested) return {ABORT_SCORE, {}};
        if (timeExceeded())      return {ABORT_SCORE, {}};

        this->nodes++;

        MoveGenerator moveGenerator;
        std::vector<std::string> moves = moveGenerator.getAllMoves(board);
        bool maximizingPlayer = board.getPlayerToMove() == 'w';

        unsigned long long myKing   = maximizingPlayer ? (board.kings & board.whites) : (board.kings & board.blacks);
        unsigned long long oppKing  = maximizingPlayer ? (board.kings & board.blacks) : (board.kings & board.whites);
        if (myKing == 0)  { int ms = CHECKMATE_SCORE - ply; return { maximizingPlayer ? -ms : ms, {} }; }
        if (oppKing == 0) { int ms = CHECKMATE_SCORE - ply; return { maximizingPlayer ?  ms : -ms, {} }; }

        char opponent = maximizingPlayer ? 'b' : 'w';
        int king_sq = -1;
        bool inCheck = false;
        if (myKing) {
                king_sq = __builtin_ffsll(myKing) - 1;
                inCheck = moveGenerator.isSquareAttacked(board, king_sq, opponent);
        }

        if (depth == 0 || moves.empty())
        {
                if (moves.empty())
                {
                        if (inCheck) {
                                int ms = CHECKMATE_SCORE - ply;
                                return { maximizingPlayer ? -ms : ms, {} };
                        }
                        return { 0, {} }; // stalemate
                }
                return { quiescence(board, alpha, beta, timeExceeded, ply), {} };
        }

        // Null-Move-Pruning
        if (!inCheck)
        {
                int R = (depth >= 6) ? 3 : 2;
                if (depth >= R + 1)
                {
                        unsigned long long myPieces = maximizingPlayer ? board.whites : board.blacks;
                        unsigned long long nonPawns = (board.knights | board.bishops | board.rooks | board.queens) & myPieces;
                        if (__builtin_popcountll(nonPawns) > 0)
                        {
                                Board nullBoard(board);
                                nullBoard.playerToMove = maximizingPlayer ? 'b' : 'w';
                                nullBoard.enPassant = "-";
                                nullBoard.halfmoves++;

                                int alphaNZ = beta - 1;
                                int betaNZ  = beta;
                                SearchResult nres = minimax(nullBoard, depth - 1 - R, alphaNZ, betaNZ, timeExceeded, ply + 1);
                                if (this->stopRequested || nres.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                                if (nres.score >= beta) return { nres.score, {} }; // fail-high
                        }
                }
        }

        // TT probe
        int alphaOrig = alpha;
        int betaOrig = beta;

        std::uint64_t key = computeZobristKey(board);
        auto it = tt.find(key);
        std::string ttBestMove;
        if (it != tt.end() && it->second.depth >= depth)
        {
                const TTEntry& e = it->second;
                if (e.flag == TTEntry::FLAG_EXACT) {
                        return {e.score, {e.bestMove}};
                } else if (e.flag == TTEntry::FLAG_ALPHA && e.score <= alpha) {
                        return {e.score, {e.bestMove}};
                } else if (e.flag == TTEntry::FLAG_BETA && e.score >= beta) {
                        return {e.score, {e.bestMove}};
                }
                ttBestMove = e.bestMove;
        }

        // Move ordering
        bool whiteToMove = maximizingPlayer;
        std::vector<std::pair<int,std::string>> ordered;
        ordered.reserve(moves.size());
        for (const auto& mv : moves)
        {
                int s = 0;
                if (!ttBestMove.empty() && mv == ttBestMove) s += 2'000'000;
                bool isCap = is_capture_move(board, mv, whiteToMove);
                if (isCap) s += 1'000'000 + mvv_lva_score(board, mv, whiteToMove);
                else if (ply < MAX_PLY) {
                        if (mv == killerMoves[ply][0]) s += 500'000;
                        else if (mv == killerMoves[ply][1]) s += 400'000;
                        int sideIdx = whiteToMove ? 0 : 1;
                        int from = Lib::getBitnumFromCoordinates(mv.substr(0,2));
                        int to   = Lib::getBitnumFromCoordinates(mv.substr(2,2));
                        s += history[sideIdx][from][to];
                }
                ordered.emplace_back(s, mv);
        }
        std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b){ return a.first > b.first; });

        if (maximizingPlayer)
        {
                int best = std::numeric_limits<int>::min();
                std::vector<std::string> bestLine;
                for (const auto& scored : ordered)
                {
                        const std::string& move = scored.second;
                        Board next(board);
                        next.executeMove(move);

                        int moveIdx = int(&scored - &ordered[0]);
                        bool cap = is_capture_move(board, move, whiteToMove);
                        SearchResult res;
                        if (!inCheck && !cap && depth >= 3 && moveIdx >= 3 && move != ttBestMove) {
                                int R = 1 + (depth >= 5);
                                res = minimax(next, depth - 1 - R, alpha, beta, timeExceeded, ply + 1);
                                if (this->stopRequested || res.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                                if (res.score > alpha) {
                                        res = minimax(next, depth - 1, alpha, beta, timeExceeded, ply + 1);
                                        if (this->stopRequested || res.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                                }
                        } else {
                                res = minimax(next, depth - 1, alpha, beta, timeExceeded, ply + 1);
                                if (this->stopRequested || res.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                        }

                        if (res.score > best)
                        {
                                best = res.score;
                                bestLine = res.moves;
                                bestLine.insert(bestLine.begin(), move);
                        }
                        if (res.score > alpha) alpha = res.score;

                        if (beta <= alpha) {
                                if (!cap && ply < MAX_PLY)
                                {
                                        if (killerMoves[ply][0] != move) {
                                                killerMoves[ply][1] = killerMoves[ply][0];
                                                killerMoves[ply][0] = move;
                                        }
                                        int sideIdx = whiteToMove ? 0 : 1;
                                        int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
                                        int to   = Lib::getBitnumFromCoordinates(move.substr(2,2));
                                        history[sideIdx][from][to] += depth * depth;
                                }
                                break;
                        }
                }
                // TT store
                {
                        TTEntry e;
                        e.key = key; e.depth = depth; e.score = best;
                        if (best <= alphaOrig) e.flag = TTEntry::FLAG_ALPHA;
                        else if (best >= betaOrig) e.flag = TTEntry::FLAG_BETA;
                        else e.flag = TTEntry::FLAG_EXACT;
                        e.bestMove = bestLine.empty() ? (ordered.empty() ? std::string() : ordered.front().second) : bestLine.front();
                        if (tt.size() > ttMaxEntries) tt.clear();
                        tt[key] = e;
                }
                return {best, bestLine};
        }
        else
        {
                int best = std::numeric_limits<int>::max();
                std::vector<std::string> bestLine;
                for (const auto& scored : ordered)
                {
                        const std::string& move = scored.second;
                        Board next(board);
                        next.executeMove(move);

                        int moveIdx = int(&scored - &ordered[0]);
                        bool cap = is_capture_move(board, move, whiteToMove);
                        SearchResult res;
                        if (!inCheck && !cap && depth >= 3 && moveIdx >= 3 && move != ttBestMove) {
                                int R = 1 + (depth >= 5);
                                res = minimax(next, depth - 1 - R, alpha, beta, timeExceeded, ply + 1);
                                if (this->stopRequested || res.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                                if (res.score < beta) {
                                        res = minimax(next, depth - 1, alpha, beta, timeExceeded, ply + 1);
                                        if (this->stopRequested || res.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                                }
                        } else {
                                res = minimax(next, depth - 1, alpha, beta, timeExceeded, ply + 1);
                                if (this->stopRequested || res.score == ABORT_SCORE) return {ABORT_SCORE, {}};
                        }

                        if (res.score < best)
                        {
                                best = res.score;
                                bestLine = res.moves;
                                bestLine.insert(bestLine.begin(), move);
                        }
                        if (res.score < beta) beta = res.score;

                        if (beta <= alpha) {
                                if (!cap && ply < MAX_PLY)
                                {
                                        if (killerMoves[ply][0] != move) {
                                                killerMoves[ply][1] = killerMoves[ply][0];
                                                killerMoves[ply][0] = move;
                                        }
                                        int sideIdx = whiteToMove ? 0 : 1;
                                        int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
                                        int to   = Lib::getBitnumFromCoordinates(move.substr(2,2));
                                        history[sideIdx][from][to] += depth * depth;
                                }
                                break;
                        }
                }
                // TT store
                {
                        TTEntry e;
                        e.key = key; e.depth = depth; e.score = best;
                        if (best <= alphaOrig) e.flag = TTEntry::FLAG_ALPHA;
                        else if (best >= betaOrig) e.flag = TTEntry::FLAG_BETA;
                        else e.flag = TTEntry::FLAG_EXACT;
                        e.bestMove = bestLine.empty() ? (ordered.empty() ? std::string() : ordered.front().second) : bestLine.front();
                        if (tt.size() > ttMaxEntries) tt.clear();
                        tt[key] = e;
                }
                return {best, bestLine};
        }
}

// --------------- go() with 3+2 time mgmt ---------------
void Engine::go(GoParams params)
{
        this->stopRequested = false;
        this->nodes = 0;

        int movetime = params.movetime;
        if (movetime <= 0)
        {
                char player = this->board.getPlayerToMove();
                int time = (player == 'w') ? params.wtime : params.btime;    // ms
                int inc  = (player == 'w') ? params.winc  : params.binc;     // ms

                int reserve = std::max(2500, time / 20); // keep at least 2.5s or 5%

                int pieceCount = __builtin_popcountll(this->board.whites | this->board.blacks);
                int mtg = (pieceCount > 16) ? 30 : (pieceCount > 8 ? 20 : 12);

                int base = time / std::max(1, mtg);
                int incShare = (inc * 6) / 10;

                long long target = base + incShare;

                long long hardCap = std::max(50, time - reserve);
                long long softCapEarly = time / 12 + 2LL * inc;
                long long softCapLate  = time / 8  + 1LL * inc;

                long long softCap = (pieceCount > 12) ? softCapEarly : softCapLate;
                target = std::min(target, softCap);
                target = std::min(target, hardCap);

                if (time < 10 * std::max(1, inc)) {
                        target = std::max( (long long)inc * 9 / 10, 30LL );
                }

                movetime = (int)std::max(30LL, std::min(target, 30000LL));
        }

        auto start = std::chrono::steady_clock::now();

        auto elapsedMs = [&](){
                auto now = std::chrono::steady_clock::now();
                return (unsigned long long) std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
        };

        auto timeExceeded = [&](){
                return elapsedMs() >= (unsigned long long)movetime;
        };

        int bestScore = 0;
        std::vector<std::string> bestPV;
        int maxDepth = 128;

        for (int depth = 1; depth <= maxDepth; ++depth)
        {
                int alpha = std::numeric_limits<int>::min() + 1;
                int beta  = std::numeric_limits<int>::max() - 1;

                SearchResult res = minimax(this->board, depth, alpha, beta, timeExceeded, /*ply*/0);
                if (res.score == ABORT_SCORE) break;

                bestScore = res.score;
                bestPV    = res.moves;

                unsigned long long ms   = elapsedMs();
                unsigned long long nps  = ms ? (this->nodes * 1000ULL) / ms : 0ULL;

                emitInfo(ms, this->nodes, nps, bestScore, bestPV);

                if (timeExceeded()) break;
        }

        std::string bestmove = bestPV.empty() ? "" : bestPV.front();
        if (bestmove.empty()) {
                MoveGenerator gen;
                auto moves = gen.getAllMoves(this->board);
                if (!moves.empty()) bestmove = moves.front();
        }
        if (bestmove.empty()) bestmove = "0000";

        this->comm->uciOutput("bestmove " + bestmove + "\n");
}

// --------------- emitInfo ---------------
void Engine::emitInfo(unsigned long long elapsed,
                      unsigned long long nodes,
                      unsigned long long nps,
                      int score,
                      const std::vector<std::string>& pv)
{
        std::ostringstream out;
        out << "info time "  << elapsed
            << " nodes "     << nodes
            << " nps "       << nps
            << " score ";

        if (std::abs(score) > CHECKMATE_SCORE / 2) {
                int m = (score > 0 ? CHECKMATE_SCORE - score : CHECKMATE_SCORE + score);
                int mateIn = (m + 1) / 2;
                if (mateIn < 0) mateIn = -mateIn;
                out << "mate " << (score > 0 ? mateIn : -mateIn);
        } else {
                out << "cp " << score;
        }

        out << " pv";
        for (const auto& m : pv) out << " " << m;
        out << "\n";

        this->comm->uciOutput(out.str());
}

// --------------- Perft wrappers ---------------
void Engine::perft(int depth)
{
        PerftResult result = this->perft_runner.perft(this->board, depth);
        std::ostringstream output;
        output << "info string [Engine::perft] nodes "      << Lib::formatThousands(result.nodes)      << std::endl
               << "info string [Engine::perft] captures "   << Lib::formatThousands(result.captures)   << std::endl
               << "info string [Engine::perft] ep "         << Lib::formatThousands(result.en_passant) << std::endl
               << "info string [Engine::perft] castles "    << Lib::formatThousands(result.castles)    << std::endl
               << "info string [Engine::perft] promotions " << Lib::formatThousands(result.promotions) << std::endl
               << "info string [Engine::perft] checks "     << Lib::formatThousands(result.checks)     << std::endl
               << "info string [Engine::perft] checkmates " << Lib::formatThousands(result.checkmates) << std::endl;
        this->comm->uciOutput(output.str());
}

unsigned long long Engine::perftNodes(int depth)
{
        PerftResult result = this->perft_runner.perft(this->board, depth);
        return result.nodes;
}

void Engine::perftDivide(int depth)
{
        this->perft_runner.perftDivide(this->board, depth);
}
