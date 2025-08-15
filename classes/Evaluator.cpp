#include "Evaluator.h"
#include "Board.h"
#include "Lib.h"
#include <iostream>

int Evaluator::evaluate(Board& board, const std::string& move)
{
	int score = 0;
	int to = Lib::getBitnumFromCoordinates(move.substr(2, 4));

	// 1. Prefer captures
	unsigned long long opponent_pieces = (board.getPlayerToMove() == 'w') ? board.blacks : board.whites;
	if ((opponent_pieces & (1ULL << to)) != 0)
	{
		score += 100;
	}

	// 2. Prefer moves to the center
	if (to == 27 || to == 28 || to == 35 || to == 36)
	{
		score += 10;
	}

	std::cout << "info string [Evaluator::evaluate] " << move << ": " << score << std::endl;
	return score;
}
