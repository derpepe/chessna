#include "MoveGenerator.h"
#include "Board.h"
#include "Lib.h"

void MoveGenerator::addMoveToList(Board& board, std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, std::string promotion)
{
	for (std::vector<int>::size_type i = 0; i < checkForEmpty.size(); i++) {
		int interim = checkForEmpty[i];
		if (((board.whites | board.blacks) & (1ULL << interim)) > 0) {
			return; // Path is blocked
		}
	}

	for (char const& p : promotion) {
		moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(to) + p);
	}
}

void MoveGenerator::addMoveToList(Board& board, std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty)
{
	if ((to > 63) || (to < 0)) {
		return;
	}

	for (std::vector<int>::size_type i = 0; i < checkForEmpty.size(); i++) {
		int interim = checkForEmpty[i];
		if (((board.whites | board.blacks) & (1ULL << interim)) > 0) {
			return; // Path is blocked
		}
	}

	unsigned long long friendly_pieces = (board.playerToMove == 'w') ? board.whites : board.blacks;
	if ((friendly_pieces & (1ULL << to)) > 0) {
		return; // Cannot capture friendly piece
	}

	moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(to));
}

void MoveGenerator::addMoveToList(Board& board, std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, int maxdist)
{
	if ((to > 63) || (to < 0)) {
		return;
	}

	if (!((abs(Lib::getRank(from) - Lib::getRank(to)) <= maxdist) && (abs(Lib::getFile(from) - Lib::getFile(to)) <= maxdist))) {
		return;
	}

	for (std::vector<int>::size_type i = 0; i < checkForEmpty.size(); i++) {
		int interim = checkForEmpty[i];
		if (((board.whites | board.blacks) & (1ULL << interim)) > 0) {
			return;
		}
	}

	if (board.playerToMove == 'w') {
		if ((board.whites & (1ULL << to)) > 0) {
			return;
		}
	} else {
		if ((board.blacks & (1ULL << to)) > 0) {
			return;
		}
	}

	moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(to));
}

void MoveGenerator::addSlidingMoves(Board& board, std::vector<std::string> *moves, int from, const std::vector<int>& directions)
{
	unsigned long long friendly_pieces = (board.playerToMove == 'w') ? board.whites : board.blacks;
	unsigned long long occupied = board.whites | board.blacks;

	for (int direction : directions) {
		int current_pos = from;
		while (true) {
			int next_pos = current_pos + direction;

			if (next_pos < 0 || next_pos > 63) break;

			int current_file = Lib::getFile(current_pos);
			int next_file = Lib::getFile(next_pos);
			if (abs(current_file - next_file) > 1) break;

			if ((1ULL << next_pos) & friendly_pieces) break;

			moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(next_pos));

			if ((1ULL << next_pos) & occupied) break;

			current_pos = next_pos;
		}
	}
}

std::vector<std::string> MoveGenerator::getAllMoves(Board& board)
{
	int from, to;
	std::vector<std::string> moves;
	std::vector<std::string> legalMoves;

	unsigned long long pieces = (board.playerToMove == 'w') ? board.whites : board.blacks;

	unsigned long long current_king = board.kings & pieces;
	if (current_king > 0) {
		from = __builtin_ffsll(current_king) - 1;
		this->addMoveToList(board, &moves, from, from + 8, {}, 1);
		this->addMoveToList(board, &moves, from, from - 8, {}, 1);
		this->addMoveToList(board, &moves, from, from + 1, {}, 1);
		this->addMoveToList(board, &moves, from, from - 1, {}, 1);
		this->addMoveToList(board, &moves, from, from + 9, {}, 1);
		this->addMoveToList(board, &moves, from, from - 9, {}, 1);
		this->addMoveToList(board, &moves, from, from + 7, {}, 1);
		this->addMoveToList(board, &moves, from, from - 7, {}, 1);

		if (board.playerToMove == 'w') {
			if (board.casteling_K && !isSquareAttacked(board, from, 'b') && !isSquareAttacked(board, from + 1, 'b') && !isSquareAttacked(board, from + 2, 'b'))
			{
				this->addMoveToList(board, &moves, from, from + 2, {from + 1});
			}
			if (board.casteling_Q && !isSquareAttacked(board, from, 'b') && !isSquareAttacked(board, from - 1, 'b') && !isSquareAttacked(board, from - 2, 'b'))
			{
				this->addMoveToList(board, &moves, from, from - 2, {from - 1, from - 2, from - 3});
			}
		} else {
			if (board.casteling_k && !isSquareAttacked(board, from, 'w') && !isSquareAttacked(board, from + 1, 'w') && !isSquareAttacked(board, from + 2, 'w'))
			{
				this->addMoveToList(board, &moves, from, from + 2, {from + 1});
			}
			if (board.casteling_q && !isSquareAttacked(board, from, 'w') && !isSquareAttacked(board, from - 1, 'w') && !isSquareAttacked(board, from - 2, 'w'))
			{
				this->addMoveToList(board, &moves, from, from - 2, {from - 1, from - 2, from - 3});
			}
		}
	}

	unsigned long long current_queens = board.queens & pieces;
	while (current_queens > 0) {
		from = __builtin_ffsll(current_queens) - 1;
		this->addSlidingMoves(board, &moves, from, {8, -8, 1, -1, 7, -7, 9, -9});
		current_queens &= ~(1ULL << from);
	}

	unsigned long long current_rooks = board.rooks & pieces;
	while (current_rooks > 0) {
		from = __builtin_ffsll(current_rooks) - 1;
		this->addSlidingMoves(board, &moves, from, {8, -8, 1, -1});
		current_rooks &= ~(1ULL << from);
	}

	unsigned long long current_bishops = board.bishops & pieces;
	while (current_bishops > 0) {
		from = __builtin_ffsll(current_bishops) - 1;
		this->addSlidingMoves(board, &moves, from, {7, -7, 9, -9});
		current_bishops &= ~(1ULL << from);
	}

	unsigned long long current_knights = board.knights & pieces;
	while (current_knights > 0) {
		from = __builtin_ffsll(current_knights) - 1;
		this->addMoveToList(board, &moves, from, from -  6, {}, 2);
		this->addMoveToList(board, &moves, from, from - 10, {}, 2);
		this->addMoveToList(board, &moves, from, from - 15, {}, 2);
		this->addMoveToList(board, &moves, from, from - 17, {}, 2);
		this->addMoveToList(board, &moves, from, from +  6, {}, 2);
		this->addMoveToList(board, &moves, from, from + 10, {}, 2);
		this->addMoveToList(board, &moves, from, from + 15, {}, 2);
		this->addMoveToList(board, &moves, from, from + 17, {}, 2);
		current_knights &= ~(1ULL << from);
	}

	unsigned long long current_pawns = board.pawns & pieces;
	while (current_pawns > 0) {
		from = __builtin_ffsll(current_pawns) - 1;
		if (board.playerToMove == 'w') {
			to = from + 8;
			if (to < 64 && !((1ULL << to) & (board.whites | board.blacks))) {
				if (Lib::getRank(from) == 6) {
					this->addMoveToList(board, &moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
			if (Lib::getRank(from) == 1) {
				to = from + 16;
				if (!((1ULL << to) & (board.whites | board.blacks)) && !((1ULL << (from + 8)) & (board.whites | board.blacks))) {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
			to = from + 7;
			if (Lib::getFile(from) > 0 && ((1ULL << to) & board.blacks)) {
				if (Lib::getRank(from) == 6) {
					this->addMoveToList(board, &moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
			to = from + 9;
			if (Lib::getFile(from) < 7 && ((1ULL << to) & board.blacks)) {
				if (Lib::getRank(from) == 6) {
					this->addMoveToList(board, &moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
		} else {
			to = from - 8;
			if (to >= 0 && !((1ULL << to) & (board.whites | board.blacks))) {
				if (Lib::getRank(from) == 1) {
					this->addMoveToList(board, &moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
			if (Lib::getRank(from) == 6) {
				to = from - 16;
				if (!((1ULL << to) & (board.whites | board.blacks)) && !((1ULL << (from - 8)) & (board.whites | board.blacks))) {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
			to = from - 7;
			if (Lib::getFile(from) < 7 && ((1ULL << to) & board.whites)) {
				if (Lib::getRank(from) == 1) {
					this->addMoveToList(board, &moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
			to = from - 9;
			if (Lib::getFile(from) > 0 && ((1ULL << to) & board.whites)) {
				if (Lib::getRank(from) == 1) {
					this->addMoveToList(board, &moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(board, &moves, from, to, {});
				}
			}
		}

		if (board.enPassant != "-") {
			int ep_square = Lib::getBitnumFromCoordinates(board.enPassant);
			if (board.playerToMove == 'w') {
				if (ep_square == from + 7 && Lib::getFile(from) > 0) this->addMoveToList(board, &moves, from, ep_square, {});
				if (ep_square == from + 9 && Lib::getFile(from) < 7) this->addMoveToList(board, &moves, from, ep_square, {});
			} else {
				if (ep_square == from - 7 && Lib::getFile(from) < 7) this->addMoveToList(board, &moves, from, ep_square, {});
				if (ep_square == from - 9 && Lib::getFile(from) > 0) this->addMoveToList(board, &moves, from, ep_square, {});
			}
		}

		current_pawns &= ~(1ULL << from);
	}

	for (std::vector<std::string>::iterator it = moves.begin(); it != moves.end(); ++it)
	{
		Board nextBoard(board);
		nextBoard.executeMove(*it, false);

		unsigned long long king_bb;
		if (board.playerToMove == 'w') {
			king_bb = nextBoard.kings & nextBoard.whites;
		} else {
			king_bb = nextBoard.kings & nextBoard.blacks;
		}
		int king_sq = __builtin_ffsll(king_bb) - 1;

		char opponent = board.playerToMove == 'w' ? 'b' : 'w';
		if (!this->isSquareAttacked(nextBoard, king_sq, opponent))
		{
			legalMoves.push_back(*it);
		}
	}

	return legalMoves;
}

bool MoveGenerator::isSquareAttacked(Board& board, int square, char byPlayer)
{
	unsigned long long occupied = board.whites | board.blacks;
	unsigned long long attackers;

	if (byPlayer == 'w') {
		attackers = board.whites;
	} else {
		attackers = board.blacks;
	}

	if (byPlayer == 'w') {
		if (Lib::getFile(square) > 0 && (board.pawns & board.whites & (1ULL << (square - 9)))) return true;
		if (Lib::getFile(square) < 7 && (board.pawns & board.whites & (1ULL << (square - 7)))) return true;
	} else {
		if (Lib::getFile(square) > 0 && (board.pawns & board.blacks & (1ULL << (square + 7)))) return true;
		if (Lib::getFile(square) < 7 && (board.pawns & board.blacks & (1ULL << (square + 9)))) return true;
	}

	unsigned long long knight_attacks = 0;
	if (square > 17 && Lib::getFile(square) > 0) knight_attacks |= (1ULL << (square - 17));
	if (square > 15 && Lib::getFile(square) < 7) knight_attacks |= (1ULL << (square - 15));
	if (square > 10 && Lib::getFile(square) > 1) knight_attacks |= (1ULL << (square - 10));
	if (square > 6 && Lib::getFile(square) < 6) knight_attacks |= (1ULL << (square - 6));
        // prevent undefined behaviour when shifting beyond 64 bits
        // squares with index >=49 would overflow when adding 15
        if (square < 49 && Lib::getFile(square) > 0) knight_attacks |= (1ULL << (square + 15));
        // squares with index >=47 would overflow when adding 17
        if (square < 47 && Lib::getFile(square) < 7) knight_attacks |= (1ULL << (square + 17));
        // squares with index >=58 would overflow when adding 6
        if (square < 58 && Lib::getFile(square) > 1) knight_attacks |= (1ULL << (square + 6));
        // squares with index >=54 would overflow when adding 10
        if (square < 54 && Lib::getFile(square) < 6) knight_attacks |= (1ULL << (square + 10));
	if (board.knights & attackers & knight_attacks) return true;

	unsigned long long king_attacks = 0;
	if (Lib::getFile(square) > 0) king_attacks |= (1ULL << (square - 1));
	if (Lib::getFile(square) < 7) king_attacks |= (1ULL << (square + 1));
	if (square > 7) king_attacks |= (1ULL << (square - 8));
	if (square < 56) king_attacks |= (1ULL << (square + 8));
	if (square > 8 && Lib::getFile(square) > 0) king_attacks |= (1ULL << (square - 9));
	if (square > 7 && Lib::getFile(square) < 7) king_attacks |= (1ULL << (square - 7));
	if (square < 56 && Lib::getFile(square) > 0) king_attacks |= (1ULL << (square + 7));
	if (square < 55 && Lib::getFile(square) < 7) king_attacks |= (1ULL << (square + 9));
	if (board.kings & attackers & king_attacks) return true;

	unsigned long long rooks_and_queens = (board.rooks | board.queens) & attackers;
	int current_rank = Lib::getRank(square);
	for (int i = square + 1; i < 64 && Lib::getRank(i) == current_rank; i++) { // Right
		if ((rooks_and_queens & (1ULL << i)) != 0) return true;
		if ((occupied & (1ULL << i)) != 0) break;
	}
	for (int i = square - 1; i >= 0 && Lib::getRank(i) == current_rank; i--) { // Left
		if ((rooks_and_queens & (1ULL << i)) != 0) return true;
		if ((occupied & (1ULL << i)) != 0) break;
	}
	for (int i = square + 8; i < 64; i += 8) { if ((rooks_and_queens & (1ULL << i)) != 0) return true; if ((occupied & (1ULL << i)) != 0) break; }
	for (int i = square - 8; i >= 0; i -= 8) { if ((rooks_and_queens & (1ULL << i)) != 0) return true; if ((occupied & (1ULL << i)) != 0) break; }

	unsigned long long bishops_and_queens = (board.bishops | board.queens) & attackers;
	const std::vector<int> directions = {7, -7, 9, -9};
	for (int direction : directions)
	{
		int current_pos = square;
		while (true)
		{
			int next_pos = current_pos + direction;
			if (next_pos < 0 || next_pos > 63) break;
			if (abs(Lib::getFile(next_pos) - Lib::getFile(current_pos)) > 1) break;

			if ((bishops_and_queens & (1ULL << next_pos)) != 0)
			{
				return true;
			}
			if ((occupied & (1ULL << next_pos)) != 0)
			{
				break;
			}
			current_pos = next_pos;
		}
	}

	return false;
}
