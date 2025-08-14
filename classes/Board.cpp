#define DEBUG

#include "Board.h"

Board::Board()
{
	this->startpos();
}

Board::Board(const Board& other)
{
	this->blacks = other.blacks;
	this->whites = other.whites;
	this->kings = other.kings;
	this->queens = other.queens;
	this->rooks = other.rooks;
	this->bishops = other.bishops;
	this->knights = other.knights;
	this->pawns = other.pawns;
	this->playerToMove = other.playerToMove;
	this->casteling_K = other.casteling_K;
	this->casteling_k = other.casteling_k;
	this->casteling_Q = other.casteling_Q;
	this->casteling_q = other.casteling_q;
	this->enPassant = other.enPassant;
	this->halfmoves = other.halfmoves;
	this->currentMove = other.currentMove;
}

void Board::startpos()
{
	this->loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}


void Board::executeMove(std::string move, bool incrementCounters)
{
	int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
	int to = Lib::getBitnumFromCoordinates(move.substr(2,2));
#ifdef DEBUG
	std::cout << "info string [Board::executeMove] moves figure from " << from << " to " << to << std::endl;
#endif
	
	// check if a figure gets captured (for potential reset of halfmoves-counter)
	bool figureCaptured = (((1ULL << to) & (this->whites | this->blacks)) != 0);

	if (figureCaptured) {
		unsigned long long capture_mask = ~(1ULL << to);
		this->rooks &= capture_mask;
		this->knights &= capture_mask;
		this->bishops &= capture_mask;
		this->queens &= capture_mask;
		this->pawns &= capture_mask;
		if (this->playerToMove == 'w') {
			this->blacks &= capture_mask;
		} else {
			this->whites &= capture_mask;
		}
	}

	this->blacks = Lib::moveBit(this->blacks, from, to);
	this->whites = Lib::moveBit(this->whites, from, to);
	this->kings = Lib::moveBit(this->kings, from, to);
	this->queens = Lib::moveBit(this->queens, from, to);
	this->rooks = Lib::moveBit(this->rooks, from, to);
	this->bishops = Lib::moveBit(this->bishops, from, to);
	this->knights = Lib::moveBit(this->knights, from, to);
	this->pawns = Lib::moveBit(this->pawns, from, to);
	
	this->checkConsistency();
	
	// update casteling if rook was moved
	if ((to == 0) || (from == 0))
	{
		casteling_Q = false;
	}
	else if ((to == 7) || (from == 7))
	{
		casteling_K = false;
	}
	else if ((to == 56) || (from == 56))
	{
		casteling_q = false;
	}
	else if ((to == 63) || (from == 63))
	{
		casteling_k = false;
	}
	
	// update casteling if king has moved and also move rook if necessary
	if ((this->kings & (1ULL << to)) != 0)
	{
		// invalidate casteling
		if ((this->whites & (1ULL << to)) != 0)
		{
			casteling_K = false;
			casteling_Q = false;
		}
		else
		{
			casteling_k = false;
			casteling_q = false;
		}
		
		if (abs(to - from) > 1) { // king moved more than 1 square -> casteling
			// casteling just happend, also move rook!
			switch(to)
			{
				case 2: // white, queenside
					this->rooks = Lib::moveBit(this->rooks, 0, 3);
					this->whites = Lib::moveBit(this->whites, 0, 3);
					break;
				case 6: // white, kingside
					this->rooks = Lib::moveBit(this->rooks, 7, 5);
					this->whites = Lib::moveBit(this->whites, 7, 5);
					break;
				case 58: // black, queenside
					this->rooks = Lib::moveBit(this->rooks, 56, 59);
					this->blacks = Lib::moveBit(this->blacks, 56, 59);
					break;
				case 62: // black, kingside
					this->rooks = Lib::moveBit(this->rooks, 63, 61);
					this->blacks = Lib::moveBit(this->blacks, 63, 61);
					break;
			}
		}
	}
	
	// en passant
	if (((1ULL << to) & this->pawns) && (abs(to - from) == 16))
	{
		// pawn moved 2 fields
		int field = -1;
		if (((1UL << to) & this->whites) != 0)
		{
			// white pawn moved
			field = to - 8;
		}
		else
		{
			// black pawn moved
			field = to + 8;
		}
		this->enPassant = Lib::getCoordinatesFromBitnum(field);
	} else {
		this->enPassant = "-";
	}
	
	// pawn promotion
	if (move.size() > 4)
	{
		char promoteTo = move.c_str()[4];
		unsigned long long to_bb = 1ULL << to;
		this->pawns &= ~to_bb; // remove from pawns
		switch (promoteTo)
		{
			case 'q': this->queens |= to_bb; break;
			case 'r': this->rooks |= to_bb; break;
			case 'n': this->knights |= to_bb; break;
			case 'b': this->bishops |= to_bb; break;
		}
	}
	
	if (incrementCounters)
	{
		this->playerToMove = this->playerToMove == 'w' ? 'b' : 'w';
		this->halfmoves++;
		if (this->playerToMove == 'w')
		{
			this->currentMove++;
		}
		if (figureCaptured || (((1ULL << to) & this->pawns) != 0))
		{
			this->halfmoves = 0;
		}
	}
}

void Board::loadFen(const std::string& fen)
{
	std::vector<std::string> token = Lib::split(fen, ' ');
	this->loadFen(token[0], token[1][0], token[2], token[3], atol(token[4].c_str()), atol(token[5].c_str()));
}

void Board::loadFen(const std::string& figures, char playerToMove, const std::string& casteling, const std::string& enPassant,
	long halfmoves, long currentMove)
{
	this->whites = 0;
	this->blacks = 0;
	this->kings = 0;
	this->queens = 0;
	this->rooks = 0;
	this->bishops = 0;
	this->knights = 0;
	this->pawns = 0;

	int x = 0;
	int y = 7;
	for (char const& c : figures)
	{
		unsigned long long bit = 1ULL << (y * 8 + x);
		switch (c)
		{
			case 'K': this->whites |= bit; this->kings |= bit; x++; break;
			case 'Q': this->whites |= bit; this->queens |= bit; x++; break;
			case 'R': this->whites |= bit; this->rooks |= bit; x++; break;
			case 'B': this->whites |= bit; this->bishops |= bit; x++; break;
			case 'N': this->whites |= bit; this->knights |= bit; x++; break;
			case 'P': this->whites |= bit; this->pawns |= bit; x++; break;

			case 'k': this->blacks |= bit; this->kings |= bit; x++; break;
			case 'q': this->blacks |= bit; this->queens |= bit; x++; break;
			case 'r': this->blacks |= bit; this->rooks |= bit; x++; break;
			case 'b': this->blacks |= bit; this->bishops |= bit; x++; break;
			case 'n': this->blacks |= bit; this->knights |= bit; x++; break;
			case 'p': this->blacks |= bit; this->pawns |= bit; x++; break;

			case '/': x = 0; y--; break;

			case '1': x += 1; break;
			case '2': x += 2; break;
			case '3': x += 3; break;
			case '4': x += 4; break;
			case '5': x += 5; break;
			case '6': x += 6; break;
			case '7': x += 7; break;
			case '8': x += 8; break;
		}
	}

	this->playerToMove = playerToMove;

	this->casteling_K = (casteling.find('K') != std::string::npos);
	this->casteling_Q = (casteling.find('Q') != std::string::npos);
	this->casteling_k = (casteling.find('k') != std::string::npos);
	this->casteling_q = (casteling.find('q') != std::string::npos);

	this->enPassant = enPassant;
	this->halfmoves = halfmoves;
	this->currentMove = currentMove;
}

std::string Board::getDump()
{
	std::string prefix = "info string ";
	std::ostringstream result;
	std::ostringstream fen;

	result << prefix << "   A B C D E F G H" << std::endl;
	result << prefix << std::endl;

	unsigned long long figures = (this->whites | this->blacks);

	int emptycount = 0;
	for (int y = 0; y < 8; y++) {
		result << prefix << (8 - y) << "  ";
		for (int x = 0; x < 8; x++) {
			std::string f = "?";
			int bit = (7 - y) * 8 + (7 - x);
			unsigned long long position = 1ULL << bit;
			if (position & figures) {
				if (position & kings) f = "K";
				if (position & queens) f = "Q";
				if (position & rooks) f = "R";
				if (position & knights) f = "N";
				if (position & bishops) f = "B";
				if (position & pawns) f = "P";
				if (position & blacks) std::transform(f.begin(), f.end(), f.begin(), ::tolower);
				if (emptycount > 0)
				{
					fen << emptycount;
					emptycount = 0;
				}
				fen << f;
			}
			else
			{
				f = "-";
				emptycount++;
			}
			result << f << ' ';
		}
		if (emptycount > 0)
		{
			fen << emptycount;
			emptycount = 0;
		}
		if (y < 7) fen << '/';
		result << "  " << (8 - y) << std::endl;
	}
	if (emptycount > 0)
	{
		fen << emptycount;
		emptycount = 0;
	}
	result << prefix << std::endl;
	result << prefix << "   A B C D E F G H" << std::endl;

	std::ostringstream casteling;
	if (this->casteling_K) casteling << 'K';
	if (this->casteling_Q) casteling << 'Q';
	if (this->casteling_k) casteling << 'k';
	if (this->casteling_q) casteling << 'q';
	if (casteling.str().length() == 0) casteling << '-';

	result << prefix << "FEN: " << fen.str()
		<< " " << this->playerToMove
		<< " " << casteling.str()
		<< " " << this->enPassant
		<< " " << this->halfmoves
		<< " " << this->currentMove
		<< std::endl;
	
	this->checkConsistency();
	
	return result.str();
}


void Board::checkConsistency()
{
	unsigned long long all = this->rooks | this->knights | this->bishops | this->queens | this->kings | this->pawns;
	unsigned long long figures = (this->whites | this->blacks);
	if (!((all == figures)
		&& (figures - this->whites - this->blacks == 0)
		&& (figures - this->rooks - this->knights - this->bishops - this->queens - this->kings - this->pawns == 0)))
	{
		std::cout << "info string [Board::checkConsistency] board information is INCONSISTENT." << std::endl;
	}
}

void Board::addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, std::string promotion)
{
	for (std::vector<int>::size_type i = 0; i < checkForEmpty.size(); i++) {
		int interim = checkForEmpty[i];
		if (((this->whites | this->blacks) & (1ULL << interim)) > 0) {
			return; // Path is blocked
		}
	}

	for (char const& p : promotion) {
		moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(to) + p);
	}
}

void Board::addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty)
{
	if ((to > 63) || (to < 0)) {
		return;
	}

	for (std::vector<int>::size_type i = 0; i < checkForEmpty.size(); i++) {
		int interim = checkForEmpty[i];
		if (((this->whites | this->blacks) & (1ULL << interim)) > 0) {
			return; // Path is blocked
		}
	}

	unsigned long long friendly_pieces = (this->playerToMove == 'w') ? this->whites : this->blacks;
	if ((friendly_pieces & (1ULL << to)) > 0) {
		return; // Cannot capture friendly piece
	}

	moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(to));
}

void Board::addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, int maxdist)
{
	if ((to > 63) || (to < 0)) {
		return;
	}
	
	if (!((abs(Lib::getRank(from) - Lib::getRank(to)) <= maxdist) && (abs(Lib::getFile(from) - Lib::getFile(to)) <= maxdist))) {
		return;
	}

	for (std::vector<int>::size_type i = 0; i < checkForEmpty.size(); i++) {
		int interim = checkForEmpty[i];
		if (((this->whites | this->blacks) & (1ULL << interim)) > 0) {
			return;
		}
	}
	
	if (this->playerToMove == 'w') {
		if ((this->whites & (1ULL << to)) > 0) {
			return;
		}
	} else {
		if ((this->blacks & (1ULL << to)) > 0) {
			return;
		}
	}
	
	moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(to));
}

void Board::addSlidingMoves(std::vector<std::string> *moves, int from, const std::vector<int>& directions)
{
	unsigned long long friendly_pieces = (this->playerToMove == 'w') ? this->whites : this->blacks;
	unsigned long long occupied = this->whites | this->blacks;

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

std::vector<std::string> Board::getAllMoves()
{
	int from, to;
	std::vector<std::string> moves;
	std::vector<std::string> legalMoves;

	unsigned long long pieces = (this->playerToMove == 'w') ? this->whites : this->blacks;
	
	unsigned long long current_king = this->kings & pieces;
	if (current_king > 0) {
		from = __builtin_ffsll(current_king) - 1;
		this->addMoveToList(&moves, from, from + 8, {}, 1);
		this->addMoveToList(&moves, from, from - 8, {}, 1);
		this->addMoveToList(&moves, from, from + 1, {}, 1);
		this->addMoveToList(&moves, from, from - 1, {}, 1);
		this->addMoveToList(&moves, from, from + 9, {}, 1);
		this->addMoveToList(&moves, from, from - 9, {}, 1);
		this->addMoveToList(&moves, from, from + 7, {}, 1);
		this->addMoveToList(&moves, from, from - 7, {}, 1);

		if (this->playerToMove == 'w') {
			if (this->casteling_K && !isSquareAttacked(from, 'b') && !isSquareAttacked(from + 1, 'b') && !isSquareAttacked(from + 2, 'b'))
			{
				this->addMoveToList(&moves, from, from + 2, {from + 1});
			}
			if (this->casteling_Q && !isSquareAttacked(from, 'b') && !isSquareAttacked(from - 1, 'b') && !isSquareAttacked(from - 2, 'b'))
			{
				this->addMoveToList(&moves, from, from - 2, {from - 1, from - 2, from - 3});
			}
		} else {
			if (this->casteling_k && !isSquareAttacked(from, 'w') && !isSquareAttacked(from + 1, 'w') && !isSquareAttacked(from + 2, 'w'))
			{
				this->addMoveToList(&moves, from, from + 2, {from + 1});
			}
			if (this->casteling_q && !isSquareAttacked(from, 'w') && !isSquareAttacked(from - 1, 'w') && !isSquareAttacked(from - 2, 'w'))
			{
				this->addMoveToList(&moves, from, from - 2, {from - 1, from - 2, from - 3});
			}
		}
	}

	unsigned long long current_queens = this->queens & pieces;
	while (current_queens > 0) {
		from = __builtin_ffsll(current_queens) - 1;
		this->addSlidingMoves(&moves, from, {8, -8, 1, -1, 7, -7, 9, -9});
		current_queens &= ~(1ULL << from);
	}
	
	unsigned long long current_rooks = this->rooks & pieces;
	while (current_rooks > 0) {
		from = __builtin_ffsll(current_rooks) - 1;
		this->addSlidingMoves(&moves, from, {8, -8, 1, -1});
		current_rooks &= ~(1ULL << from);
	}

	unsigned long long current_bishops = this->bishops & pieces;
	while (current_bishops > 0) {
		from = __builtin_ffsll(current_bishops) - 1;
		this->addSlidingMoves(&moves, from, {7, -7, 9, -9});
		current_bishops &= ~(1ULL << from);
	}

	unsigned long long current_knights = this->knights & pieces;
	while (current_knights > 0) {
		from = __builtin_ffsll(current_knights) - 1;
		this->addMoveToList(&moves, from, from -  6, {}, 2);
		this->addMoveToList(&moves, from, from - 10, {}, 2);
		this->addMoveToList(&moves, from, from - 15, {}, 2);
		this->addMoveToList(&moves, from, from - 17, {}, 2);
		this->addMoveToList(&moves, from, from +  6, {}, 2);
		this->addMoveToList(&moves, from, from + 10, {}, 2);
		this->addMoveToList(&moves, from, from + 15, {}, 2);
		this->addMoveToList(&moves, from, from + 17, {}, 2);
		current_knights &= ~(1ULL << from);
	}
	
	unsigned long long current_pawns = this->pawns & pieces;
	while (current_pawns > 0) {
		from = __builtin_ffsll(current_pawns) - 1;
		if (this->playerToMove == 'w') {
			to = from + 8;
			if (to < 64 && !((1ULL << to) & (this->whites | this->blacks))) {
				if (Lib::getRank(from) == 6) {
					this->addMoveToList(&moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(&moves, from, to, {});
				}
			}
			if (Lib::getRank(from) == 1) {
				to = from + 16;
				if (!((1ULL << to) & (this->whites | this->blacks)) && !((1ULL << (from + 8)) & (this->whites | this->blacks))) {
					this->addMoveToList(&moves, from, to, {});
				}
			}
			to = from + 7;
			if (Lib::getFile(from) > 0 && ((1ULL << to) & this->blacks)) {
				if (Lib::getRank(from) == 6) {
					this->addMoveToList(&moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(&moves, from, to, {});
				}
			}
			to = from + 9;
			if (Lib::getFile(from) < 7 && ((1ULL << to) & this->blacks)) {
				if (Lib::getRank(from) == 6) {
					this->addMoveToList(&moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(&moves, from, to, {});
				}
			}
		} else {
			to = from - 8;
			if (to >= 0 && !((1ULL << to) & (this->whites | this->blacks))) {
				if (Lib::getRank(from) == 1) {
					this->addMoveToList(&moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(&moves, from, to, {});
				}
			}
			if (Lib::getRank(from) == 6) {
				to = from - 16;
				if (!((1ULL << to) & (this->whites | this->blacks)) && !((1ULL << (from - 8)) & (this->whites | this->blacks))) {
					this->addMoveToList(&moves, from, to, {});
				}
			}
			to = from - 7;
			if (Lib::getFile(from) < 7 && ((1ULL << to) & this->whites)) {
				if (Lib::getRank(from) == 1) {
					this->addMoveToList(&moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(&moves, from, to, {});
				}
			}
			to = from - 9;
			if (Lib::getFile(from) > 0 && ((1ULL << to) & this->whites)) {
				if (Lib::getRank(from) == 1) {
					this->addMoveToList(&moves, from, to, {}, "qnrb");
				} else {
					this->addMoveToList(&moves, from, to, {});
				}
			}
		}

		if (this->enPassant != "-") {
			int ep_square = Lib::getBitnumFromCoordinates(this->enPassant);
			if (this->playerToMove == 'w') {
				if (ep_square == from + 7 && Lib::getFile(from) > 0) this->addMoveToList(&moves, from, ep_square, {});
				if (ep_square == from + 9 && Lib::getFile(from) < 7) this->addMoveToList(&moves, from, ep_square, {});
			} else {
				if (ep_square == from - 7 && Lib::getFile(from) < 7) this->addMoveToList(&moves, from, ep_square, {});
				if (ep_square == from - 9 && Lib::getFile(from) > 0) this->addMoveToList(&moves, from, ep_square, {});
			}
		}
		
		current_pawns &= ~(1ULL << from);
	}
	
	for (std::vector<std::string>::iterator it = moves.begin(); it != moves.end(); ++it)
	{
		Board nextBoard(*this);
		nextBoard.executeMove(*it, false);

		unsigned long long king_bb;
		if (this->playerToMove == 'w') {
			king_bb = nextBoard.kings & nextBoard.whites;
		} else {
			king_bb = nextBoard.kings & nextBoard.blacks;
		}
		int king_sq = __builtin_ffsll(king_bb) - 1;

		char opponent = this->playerToMove == 'w' ? 'b' : 'w';
		if (!nextBoard.isSquareAttacked(king_sq, opponent))
		{
			legalMoves.push_back(*it);
		}
	}

	return legalMoves;
}

bool Board::isSquareAttacked(int square, char byPlayer)
{
	unsigned long long occupied = this->whites | this->blacks;
	unsigned long long attackers;

	if (byPlayer == 'w') {
		attackers = this->whites;
	} else {
		attackers = this->blacks;
	}

	if (byPlayer == 'w') {
		if (Lib::getFile(square) > 0 && (pawns & whites & (1ULL << (square - 9)))) return true;
		if (Lib::getFile(square) < 7 && (pawns & whites & (1ULL << (square - 7)))) return true;
	} else {
		if (Lib::getFile(square) > 0 && (pawns & blacks & (1ULL << (square + 7)))) return true;
		if (Lib::getFile(square) < 7 && (pawns & blacks & (1ULL << (square + 9)))) return true;
	}

	unsigned long long knight_attacks = 0;
	if (square > 17 && Lib::getFile(square) > 0) knight_attacks |= (1ULL << (square - 17));
	if (square > 15 && Lib::getFile(square) < 7) knight_attacks |= (1ULL << (square - 15));
	if (square > 10 && Lib::getFile(square) > 1) knight_attacks |= (1ULL << (square - 10));
	if (square > 6 && Lib::getFile(square) < 6) knight_attacks |= (1ULL << (square - 6));
	if (square < 57 && Lib::getFile(square) > 0) knight_attacks |= (1ULL << (square + 15));
	if (square < 55 && Lib::getFile(square) < 7) knight_attacks |= (1ULL << (square + 17));
	if (square < 54 && Lib::getFile(square) > 1) knight_attacks |= (1ULL << (square + 6));
	if (square < 48 && Lib::getFile(square) < 6) knight_attacks |= (1ULL << (square + 10));
	if (knights & attackers & knight_attacks) return true;

	unsigned long long king_attacks = 0;
	if (Lib::getFile(square) > 0) king_attacks |= (1ULL << (square - 1));
	if (Lib::getFile(square) < 7) king_attacks |= (1ULL << (square + 1));
	if (square > 7) king_attacks |= (1ULL << (square - 8));
	if (square < 56) king_attacks |= (1ULL << (square + 8));
	if (square > 8 && Lib::getFile(square) > 0) king_attacks |= (1ULL << (square - 9));
	if (square > 7 && Lib::getFile(square) < 7) king_attacks |= (1ULL << (square - 7));
	if (square < 56 && Lib::getFile(square) > 0) king_attacks |= (1ULL << (square + 7));
	if (square < 55 && Lib::getFile(square) < 7) king_attacks |= (1ULL << (square + 9));
	if (kings & attackers & king_attacks) return true;

	unsigned long long rooks_and_queens = (rooks | queens) & attackers;
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

	unsigned long long bishops_and_queens = (bishops | queens) & attackers;
	for (int i = square + 9; i < 64 && Lib::getFile(i) > Lib::getFile(square); i += 9) { if ((bishops_and_queens & (1ULL << i)) != 0) return true; if ((occupied & (1ULL << i)) != 0) break; }
	for (int i = square - 9; i >= 0 && Lib::getFile(i) < Lib::getFile(square); i -= 9) { if ((bishops_and_queens & (1ULL << i)) != 0) return true; if ((occupied & (1ULL << i)) != 0) break; }
	for (int i = square + 7; i < 64 && Lib::getFile(i) < Lib::getFile(square); i += 7) { if ((bishops_and_queens & (1ULL << i)) != 0) return true; if ((occupied & (1ULL << i)) != 0) break; }
	for (int i = square - 7; i >= 0 && Lib::getFile(i) > Lib::getFile(square); i -= 7) { if ((bishops_and_queens & (1ULL << i)) != 0) return true; if ((occupied & (1ULL << i)) != 0) break; }

	return false;
}

unsigned long long Board::perft(int depth)
{
	if (depth == 0)
	{
		return 1;
	}

	std::vector<std::string> moves = this->getAllMoves();
	unsigned long long nodes = 0;

	for (std::vector<std::string>::iterator it = moves.begin(); it != moves.end(); ++it)
	{
		Board nextBoard(*this);
		nextBoard.executeMove(*it);
		nodes += nextBoard.perft(depth - 1);
	}

	return nodes;
}
