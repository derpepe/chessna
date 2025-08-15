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
                        // TODO: extract this logic to a separate helper or so
                        int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
                        int to = Lib::getBitnumFromCoordinates(move.substr(2,2));
                        unsigned long long from_bb = 1ULL << from;
                        unsigned long long to_bb = 1ULL << to;

                        // Capture check before executing the move. Using the
                        // board state *after* execution would remove the
                        // captured piece from the bitboards and therefore
                        // always yield zero captures.
                        unsigned long long opponent_pieces =
                                (board.getPlayerToMove() == 'w') ? board.blacks : board.whites;
                        if ((to_bb & opponent_pieces) != 0) {
                                result.captures++;
                        }

                        // execute move for further checks
                        Board nextBoard(board);
                        nextBoard.executeMove(move);

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

        const std::string prefix = "info string [Perft::perftDivide]";
        for (const auto& move : moves)
        {
                // Determine root move characteristics
                int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
                int to = Lib::getBitnumFromCoordinates(move.substr(2,2));
                unsigned long long from_bb = 1ULL << from;
                unsigned long long to_bb = 1ULL << to;

                unsigned long long root_captures = 0;
                unsigned long long root_ep = 0;
                unsigned long long root_castles = 0;
                unsigned long long root_promotions = 0;
                unsigned long long root_checks = 0;
                unsigned long long root_checkmates = 0;

                unsigned long long opponent_pieces =
                        (board.getPlayerToMove() == 'w') ? board.blacks : board.whites;
                if ((to_bb & opponent_pieces) != 0) {
                        root_captures++;
                }
                if (((from_bb & board.pawns) != 0) && (board.enPassant != "-")) {
                        int ep_square = Lib::getBitnumFromCoordinates(board.enPassant);
                        if (to == ep_square) {
                                root_ep++;
                                root_captures++;
                        }
                }
                if (((from_bb & board.kings) != 0) && abs(to - from) == 2) {
                        root_castles++;
                }
                if (move.length() > 4) {
                        root_promotions++;
                }

                Board nextBoard(board);
                nextBoard.executeMove(move);

                // Check and checkmate detection for root move
                char opponent_color = nextBoard.getPlayerToMove();
                unsigned long long king_bb;
                if (opponent_color == 'w') {
                        king_bb = nextBoard.kings & nextBoard.whites;
                } else {
                        king_bb = nextBoard.kings & nextBoard.blacks;
                }
                int king_sq = __builtin_ffsll(king_bb) - 1;
                if (king_sq >= 0 && moveGenerator.isSquareAttacked(nextBoard, king_sq, board.getPlayerToMove())) {
                        root_checks++;
                        if (moveGenerator.getAllMoves(nextBoard).empty()) {
                                root_checkmates++;
                        }
                }

                // Results of the subtree
                PerftResult child_result = this->perft(nextBoard, depth - 1);
                child_result.captures += root_captures;
                child_result.en_passant += root_ep;
                child_result.castles += root_castles;
                child_result.promotions += root_promotions;
                child_result.checks += root_checks;
                child_result.checkmates += root_checkmates;

                total_result += child_result;
                std::cout << prefix << " " << move << ": "
                        << "nodes " << child_result.nodes << " "
                        << "captures " << child_result.captures << " "
                        << "ep " << child_result.en_passant << " "
                        << "castles " << child_result.castles << " "
                        << "promotions " << child_result.promotions << " "
                        << "checks " << child_result.checks << " "
                        << "checkmates " << child_result.checkmates
                        << std::endl;
        }

        std::cout << prefix << std::endl
                << prefix << " Total:" << std::endl
                << prefix << " nodes " << total_result.nodes << std::endl
                << prefix << " captures " << total_result.captures << std::endl
                << prefix << " ep " << total_result.en_passant << std::endl
                << prefix << " castles " << total_result.castles << std::endl
                << prefix << " promotions " << total_result.promotions << std::endl
                << prefix << " checks " << total_result.checks << std::endl
                << prefix << " checkmates " << total_result.checkmates << std::endl;
}
