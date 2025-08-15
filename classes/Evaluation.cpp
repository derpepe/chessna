#include "Evaluation.h"
#include "Board.h"
#include "Lib.h"
#include <iostream>

int Evaluation::evaluate(Board& board, const std::string& move)
{
        if (board.getPlayerToMove() == 'w')
        {
                return evaluateColorless(board, move);
        }

        Board flipped(board);
        unsigned long long temp = flipped.whites;
        flipped.whites = flipped.blacks;
        flipped.blacks = temp;

        return evaluateColorless(flipped, move);
}

int Evaluation::evaluateColorless(Board& board, const std::string& move)
{
        int score = 0;

        // 0. Evaluate material on the board
        int whiteMaterial =
                100   * __builtin_popcountll(board.pawns   & board.whites) +
                320   * __builtin_popcountll(board.knights & board.whites) +
                330   * __builtin_popcountll(board.bishops & board.whites) +
                500   * __builtin_popcountll(board.rooks   & board.whites) +
                900   * __builtin_popcountll(board.queens  & board.whites) +
                20000 * __builtin_popcountll(board.kings   & board.whites);

        int blackMaterial =
                100   * __builtin_popcountll(board.pawns   & board.blacks) +
                320   * __builtin_popcountll(board.knights & board.blacks) +
                330   * __builtin_popcountll(board.bishops & board.blacks) +
                500   * __builtin_popcountll(board.rooks   & board.blacks) +
                900   * __builtin_popcountll(board.queens  & board.blacks) +
                20000 * __builtin_popcountll(board.kings   & board.blacks);

        score += whiteMaterial - blackMaterial;

        int from = Lib::getBitnumFromCoordinates(move.substr(0, 2));
        int to = Lib::getBitnumFromCoordinates(move.substr(2, 4));

        // 1. Prefer captures weighted by piece value
        unsigned long long to_bb = 1ULL << to;
        if ((board.blacks & to_bb) != 0)
        {
                int value = 0;
                if ((board.pawns & to_bb) != 0)
                {
                        value = 100;
                }
                else if ((board.knights & to_bb) != 0)
                {
                        value = 320;
                }
                else if ((board.bishops & to_bb) != 0)
                {
                        value = 330;
                }
                else if ((board.rooks & to_bb) != 0)
                {
                        value = 500;
                }
                else if ((board.queens & to_bb) != 0)
                {
                        value = 900;
                }
                else if ((board.kings & to_bb) != 0)
                {
                        value = 20000;
                }

                score += value;
        }

        // 2. Prefer moves to the center for pawns only
        if ((board.pawns & (1ULL << from)) != 0)
        {
                if (to == 27 || to == 28 || to == 35 || to == 36)
                {
                        score += 10;
                }
        }

//        std::cout << "info string [Evaluation::evaluateColorless] " << move << ": " << score << std::endl;
        return score;
}
