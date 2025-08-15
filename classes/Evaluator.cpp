#include "Evaluator.h"
#include "Board.h"
#include "Lib.h"

int Evaluator::evaluate(Board& board, const std::string& move)
{
        int score = 0;
        int from = Lib::getBitnumFromCoordinates(move.substr(0, 2));
        int to = Lib::getBitnumFromCoordinates(move.substr(2, 4));

        // 1. Prefer captures weighted by piece value
        unsigned long long to_bb = 1ULL << to;
        unsigned long long opponent_pieces = (board.getPlayerToMove() == 'w') ? board.blacks : board.whites;
        if ((opponent_pieces & to_bb) != 0)
        {
                if ((board.pawns & to_bb) != 0)
                {
                        score += 100;
                }
                else if ((board.knights & to_bb) != 0)
                {
                        score += 320;
                }
                else if ((board.bishops & to_bb) != 0)
                {
                        score += 330;
                }
                else if ((board.rooks & to_bb) != 0)
                {
                        score += 500;
                }
                else if ((board.queens & to_bb) != 0)
                {
                        score += 900;
                }
                else if ((board.kings & to_bb) != 0)
                {
                        score += 20000;
                }
        }

        // 2. Prefer moves to the center for pawns only
        if ((board.pawns & (1ULL << from)) != 0)
        {
                if (to == 27 || to == 28 || to == 35 || to == 36)
                {
                        score += 10;
                }
        }

        return score;
}
