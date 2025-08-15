#include "Perft.h"
#include "Board.h"
#include "Lib.h"
#include "MoveGenerator.h"
#include <iostream>
#include <vector>

PerftResult Perft::perft(Board& board, int depth)
{
	PerftResult result;
	if (depth == 0)
	{
		result.nodes = 1;
		return result;
	}

	MoveGenerator moveGenerator;
	std::vector<std::string> moves = moveGenerator.getAllMoves(board);

	if (depth == 1)
	{
		result.nodes = moves.size();
		for (const auto& move : moves)
		{
			Board nextBoard(board);
			nextBoard.executeMove(move);

			// TODO: extract this logic to a separate helper or so
			int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
			int to = Lib::getBitnumFromCoordinates(move.substr(2,2));
			unsigned long long from_bb = 1ULL << from;
			unsigned long long to_bb = 1ULL << to;

			// Capture check
			unsigned long long opponent_pieces = (board.getPlayerToMove() == 'w') ? nextBoard.blacks : nextBoard.whites;
			if ((to_bb & opponent_pieces) != 0) {
				result.captures++;
			}
			// En Passant check
			if (((from_bb & board.pawns) != 0) && (board.enPassant != "-")) {
				int ep_square = Lib::getBitnumFromCoordinates(board.enPassant);
				if (to == ep_square) {
					result.en_passant++;
					result.captures++;
				}
			}
			// Castle check
			if (((from_bb & board.kings) != 0) && abs(to - from) == 2) {
				result.castles++;
			}
			// Promotion check
			if (move.length() > 4) {
				result.promotions++;
			}

			// Check and Checkmate check
			char opponent_color = nextBoard.getPlayerToMove();
			unsigned long long king_bb;
			if (opponent_color == 'w') {
				king_bb = nextBoard.kings & nextBoard.whites;
			} else {
				king_bb = nextBoard.kings & nextBoard.blacks;
			}
			int king_sq = __builtin_ffsll(king_bb) - 1;

			if (king_sq >= 0 && moveGenerator.isSquareAttacked(nextBoard, king_sq, board.getPlayerToMove())) {
				result.checks++;
				if (moveGenerator.getAllMoves(nextBoard).empty()) {
					result.checkmates++;
				}
			}
		}
	}
	else
	{
		for (const auto& move : moves)
		{
			Board nextBoard(board);
			nextBoard.executeMove(move);
			result += this->perft(nextBoard, depth - 1);
		}
	}

	return result;
}

void Perft::perftDivide(Board& board, int depth)
{
	MoveGenerator moveGenerator;
	std::vector<std::string> moves = moveGenerator.getAllMoves(board);
	PerftResult total_result;

	for (const auto& move : moves)
	{
		Board nextBoard(board);
		nextBoard.executeMove(move);
		PerftResult child_result = this->perft(nextBoard, depth - 1);
		total_result += child_result;
		std::cout << move << ": "
			<< "nodes " << child_result.nodes << " "
			<< "captures " << child_result.captures << " "
			<< "ep " << child_result.en_passant << " "
			<< "castles " << child_result.castles << " "
			<< "promotions " << child_result.promotions << " "
			<< "checks " << child_result.checks << " "
			<< "checkmates " << child_result.checkmates
			<< std::endl;
	}

	std::cout << std::endl
		<< "Total:" << std::endl
		<< " nodes " << total_result.nodes << std::endl
		<< " captures " << total_result.captures << std::endl
		<< " ep " << total_result.en_passant << std::endl
		<< " castles " << total_result.castles << std::endl
		<< " promotions " << total_result.promotions << std::endl
		<< " checks " << total_result.checks << std::endl
		<< " checkmates " << total_result.checkmates << std::endl;
}
