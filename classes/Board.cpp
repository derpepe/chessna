#include "Board.h"

Board::Board()
{
	this->startpos();
}

void Board::startpos()
{
	this->blacks  = 0xffff000000000000;
	this->whites  = 0x000000000000ffff;
	
	this->kings   = 0x0800000000000008;
	this->queens  = 0x1000000000000010;
	this->rooks   = 0x8100000000000081;
	this->bishops = 0x2400000000000024;
	this->knights = 0x4200000000000042;
	this->pawns   = 0x00ff00000000ff00;
	
	this->playerToMove = 'w';

	this->casteling_K = true;
	this->casteling_Q = true;
	this->casteling_k = true;
	this->casteling_q = true;

	this->enPassant = "-";
	
	this->halfmoves = 0;
	this->currentMove = 1;
}


void Board::executeMove(std::string move, bool incrementCounters)
{
	int from = Lib::getBitnumFromCoordinates(move.substr(0,2));
	int to = Lib::getBitnumFromCoordinates(move.substr(2,2));
	std::cout << "info string [Board::executeMove] moves figure from " << from << " to " << to << std::endl;
	
	// check if a figure gets captured (for potential reset of halfmoves-counter)
	bool figureCaptured = (((1ULL << to) & (this->whites | this->blacks)) != 0);

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
		std::cout << "info string [Board::executeMove] h1. Invlaidate casteling." << std::endl;
		casteling_K = false;
	}
	else if ((to == 7) || (from == 7))
	{
		std::cout << "info string [Board::executeMove] a1. Invlaidate casteling." << std::endl;
		casteling_Q = false;
	}
	else if ((to == 56) || (from == 56))
	{
		std::cout << "info string [Board::executeMove] h8. Invlaidate casteling." << std::endl;
		casteling_k = false;
	}
	else if ((to == 63) || (from == 63))
	{
		std::cout << "info string [Board::executeMove] a8. Invlaidate casteling." << std::endl;
		casteling_q = false;
	}
	
	// update casteling if king has moved and also move rook if necessary
	if ((this->kings & (1ULL << to)) != 0)
	{
		// invalidate casteling
		if ((this->whites & (1ULL << to)) != 0)
		{
			std::cout << "info string [Board::executeMove] White king moved. Invlaidate casteling." << std::endl;
			casteling_K = false;
			casteling_Q = false;
		}
		else
		{
			std::cout << "info string [Board::executeMove] Black king moved. Invlaidate casteling." << std::endl;
			casteling_k = false;
			casteling_q = false;
		}
		
		if (abs(to - from) > 1) {
			// casteling just happend, also move rook!
			std::cout << "info string [Board::executeMove] casteling found. Also moving rook." << std::endl;
			switch(to)
			{
				case 1:
					casteling_K = false;
					casteling_Q = false;
					this->executeMove("h1f1", false);
					break;
				case 6:
					casteling_K = false;
					casteling_Q = false;
					this->executeMove("a1c1", false);
					break;
				case 57:
					casteling_k = false;
					casteling_q = false;
					this->executeMove("h8f8", false);
					break;
				case 62:
					casteling_k = false;
					casteling_q = false;
					this->executeMove("a8c8", false);
					break;
			}
		}
	}
	
	// en passant
	if (((1ULL << to) & this->pawns) && (abs(to - from) == 16))
	{
		// pawn moved 2 fields
		std::cout << "info string [Board::executeMove] pawn moved 2 fields" << std::endl;
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
		// TODO: Set en passant field to 'field'
	}
	
	// pawn promotion
	if (move.size() > 4)
	{
		char promoteTo = move.c_str()[4];
		std::cout << "info string [Board::executeMove] Pawn promotion detected. Promoting to '" << promoteTo << "'" << std::endl;
		unsigned long long to_bb = 1ULL << to;
		this->pawns = (~to_bb) & this->pawns; // remove from pawns
		switch (promoteTo)
		{
			case 'q':
				this->queens |= to_bb;
				break;
			case 'r':
				this->rooks |= to_bb;
				break;
			case 'n':
				this->knights |= to_bb;
				break;
			case 'b':
				this->bishops |= to_bb;
				break;
		}
	}
	
	if (incrementCounters)
	{
		// update player
		this->playerToMove = this->playerToMove == 'w' ? 'b' : 'w';
	
		// update moves counter
		this->halfmoves++;
		if (this->playerToMove == 'w') // increment after move of black
		{
			this->currentMove++;
		}

		// potential reset of halfmoves-counter
		if (figureCaptured || (((1ULL << to) & this->pawns) != 0))
		{
			// pawn moved or figure captured
			std::cout << "info string [Board::executeMove] resetting halfmoves, figureCaptured = " << figureCaptured << std::endl;			
			this->halfmoves = 0;
			
		}
	}
}

void Board::loadFen(std::string fen)
{
	std::cout << "info string [Board::loadFen] lazy call: '" << fen << "'" << std::endl;
	std::vector<std::string> token = Lib::split(fen, ' ');
	this->loadFen(token[0], token[1][0], token[2], token[3], atol(token[4].c_str()), atol(token[5].c_str()));
}

void Board::loadFen(std::string figures, char playerToMove, std::string casteling, std::string enPassant,
	long halfmoves, long currentMove)
{
	std::cout << "info string [Board::loadFen] figures: " << figures << std::endl;
	std::cout << "info string [Board::loadFen] playerToMove: " << playerToMove << std::endl;
	std::cout << "info string [Board::loadFen] casteling: " << casteling << std::endl;
	std::cout << "info string [Board::loadFen] enPassant: " << enPassant << std::endl;
	std::cout << "info string [Board::loadFen] halfmoves: " << halfmoves << std::endl;
	std::cout << "info string [Board::loadFen] currentMove: " << currentMove << std::endl;

	// TODO: load figures from parameter string
	this->startpos(); // <-- to be removed

	this->playerToMove = playerToMove;

	// TODO: load casteling from parameter string

	this->enPassant = enPassant;
	// TODO: verify enPassant variable

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

	// casteling for FEN
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
	std::string prefix = "info string ";

	unsigned long long all = this->rooks | this->knights | this->bishops | this->queens | this->kings | this->pawns;
	unsigned long long figures = (this->whites | this->blacks);
	if ((all == figures)
		&& (figures - this->whites - this->blacks == 0)
		&& (figures - this->rooks - this->knights - this->bishops - this->queens - this->kings - this->pawns == 0))
	{
		std::cout << prefix << "[Board::checkConsistency] board information is consistent." << std::endl;
	}
	else
	{
		std::cout << prefix << "[Board::checkConsistency] board information is INCONSISTENT." << std::endl;
	}
}

void Board::addMoveToList(std::vector<std::string> *moves, int from, int to) {
	this->addMoveToList(moves, from, to, {}, 7);
}


void Board::addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty)
{
	this->addMoveToList(moves, from, to, checkForEmpty, 7);
}


void Board::addMoveToListDiag(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty)
{
	if ((Lib::getFile(from) == Lib::getFile(to)) && (Lib::getRank(from) == Lib::getRank(to))) {
		std::cout << "info string [Board::addMoveToListDiag] rank and file pass for diag" << std::endl;
		this->addMoveToList(moves, from, to, checkForEmpty, 7);
	} else {
		std::cout << "info string [Board::addMoveToListDiag] cannot move to " << to << " (rank and file don't pass for diag)" << std::endl;
		return;
	}
}


void Board::addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, int maxdist)
{
	// check for out of board
	if ((to > 63) || (to < 0)) {
		std::cout << "info string [Board::addMoveToList] cannot move to " << to << " (out of board)" << std::endl;
		return; // no changes
	}
	
	// check for maxdist
	if (!((abs(Lib::getRank(from) - Lib::getRank(to)) <= maxdist) && (abs(Lib::getFile(from) - Lib::getFile(to)) <= maxdist))) {
		std::cout << "info string [Board::addMoveToList] cannot move to " << Lib::getCoordinatesFromBitnum(to) << " (too far away)" << std::endl;
		return;
	} else {
		std::cout << "info string [Board::addMoveToList] distance of rank is " << (abs(Lib::getRank(from) - Lib::getRank(to))) << ", distance of file is " << (abs(Lib::getFile(from) - Lib::getFile(to))) << ", maxdist is " << maxdist << std::endl;
	}
	
	// check for occupied target-fields
	for (int i = 0; i < checkForEmpty.size(); i++) {
		int interim = checkForEmpty[i];
		if (((this->whites | this->blacks) & (1ULL << interim)) > 0) {
			std::cout << "info string [Board::addMoveToList] cannot move to " << Lib::getCoordinatesFromBitnum(to) << " (" << Lib::getCoordinatesFromBitnum(interim) << " is occupied)" << std::endl;
			return; // no changes
		} else {
			std::cout << "info string [Board::addMoveToList] " << Lib::getCoordinatesFromBitnum(to) << " is free" << std::endl;
		}
	}
	
	// check for same-colored piece at target field
	if (this->playerToMove == 'w') {
		if ((this->whites & (1ULL << to)) > 0) {
			std::cout << "info string [Board::addMoveToList] cannot move to " << Lib::getCoordinatesFromBitnum(to) << " (same-colored piece)" << std::endl;
			return; // no changes
		}
	} else {
		if ((this->blacks & (1ULL << to)) > 0) {
			std::cout << "info string [Board::addMoveToList] cannot move to " << Lib::getCoordinatesFromBitnum(to) << " (same-colored piece)" << std::endl;
			return; // no changes
		}
	}
	
	// output stats and add move
	std::cout << "info string [Board::addMoveToList] can move to " << Lib::getCoordinatesFromBitnum(to) << std::endl;
	std::cout << "info currmove " << Lib::getCoordinatesFromBitnum(from) << Lib::getCoordinatesFromBitnum(to) << std::endl; // TODO: possibily move to UCI.cpp
	moves->push_back(Lib::getCoordinatesFromBitnum(from) + Lib::getCoordinatesFromBitnum(to));
	return;
}

std::vector<std::string> Board::getAllMoves()
{
	int from, to; // commonly used for all pieces
	std::vector<std::string> moves;

	// set pieces to current color
	unsigned long long pieces;
	if (this->playerToMove == 'w') {
		pieces = this->whites;
	} else {
		pieces = this->blacks;
	}
	

	// king
	unsigned long long current_king = this->kings & pieces;
	std::cout << "info string [Board::getAllMoves] KING ------------------------------" << std::endl;
	from = __builtin_ffsll(current_king) - 1;
	std::cout << "info string [Board::getAllMoves] king at " << Lib::getCoordinatesFromBitnum(from) << std::endl;
	this->addMoveToList(&moves, from, from + 8, {}, 1);
	this->addMoveToList(&moves, from, from - 8, {}, 1);
	this->addMoveToList(&moves, from, from + 1, {}, 1);
	this->addMoveToList(&moves, from, from - 1, {}, 1);
	this->addMoveToList(&moves, from, from + 9, {}, 1);
	this->addMoveToList(&moves, from, from - 9, {}, 1);
	this->addMoveToList(&moves, from, from + 7, {}, 1);
	this->addMoveToList(&moves, from, from - 7, {}, 1);
	// TODO: check for check and mate

	// TODO: casteling moves


	// queens
	unsigned long long current_queens = this->queens & pieces;
	std::cout << "info string [Board::getAllMoves] QUEENS ------------------------------" << std::endl;
	from = __builtin_ffsll(current_queens) - 1;
	while (current_queens > 0) {
		from = __builtin_ffsll(current_queens) - 1;
		std::cout << "info string [Board::getAllMoves] queen at " << Lib::getCoordinatesFromBitnum(from) << std::endl;

		this->addMoveToList(&moves, from, from +  8, {});
		this->addMoveToList(&moves, from, from + 16, { from + 8 });
		this->addMoveToList(&moves, from, from + 24, { from + 8, from + 16 });
		this->addMoveToList(&moves, from, from + 32, { from + 8, from + 16, from + 24 });
		this->addMoveToList(&moves, from, from + 40, { from + 8, from + 16, from + 24, from + 32 });
		this->addMoveToList(&moves, from, from + 48, { from + 8, from + 16, from + 24, from + 32, from + 40 });
		this->addMoveToList(&moves, from, from + 56, { from + 8, from + 16, from + 24, from + 32, from + 40, from + 48 });

		this->addMoveToList(&moves, from, from -  8, {});
		this->addMoveToList(&moves, from, from - 16, { from - 8 });
		this->addMoveToList(&moves, from, from - 24, { from - 8, from - 16 });
		this->addMoveToList(&moves, from, from - 32, { from - 8, from - 16, from - 24 });
		this->addMoveToList(&moves, from, from - 40, { from - 8, from - 16, from - 24, from - 32 });
		this->addMoveToList(&moves, from, from - 48, { from - 8, from - 16, from - 24, from - 32, from - 40 });
		this->addMoveToList(&moves, from, from - 56, { from - 8, from - 16, from - 24, from - 32, from - 40, from - 48 });

		int rank = Lib::getRank(from);
		if (Lib::getRank(from + 1) == rank) {
			this->addMoveToList(&moves, from, from + 1, {});
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 1) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 2) == rank) {
			this->addMoveToList(&moves, from, from + 2, { from + 1 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 2) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 3) == rank) {
			this->addMoveToList(&moves, from, from + 3, { from + 1, from + 2 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 3) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 4) == rank) {
			this->addMoveToList(&moves, from, from + 4, { from + 1, from + 2, from + 3 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 4) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 5) == rank) {
			this->addMoveToList(&moves, from, from + 5, { from + 1, from + 2, from + 3, from + 4 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 5) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 6) == rank) {
			this->addMoveToList(&moves, from, from + 6, { from + 1, from + 2, from + 3, from + 4, from + 5 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 6) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 7) == rank) {
			this->addMoveToList(&moves, from, from + 7, { from + 1, from + 2, from + 3, from + 4, from + 5, from + 6 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 7) << " (not same rank)" << std::endl;
		}


		if (Lib::getRank(from - 1) == rank) {
			this->addMoveToList(&moves, from, from - 1, {});
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 1) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 2) == rank) {
			this->addMoveToList(&moves, from, from - 2, { from - 1 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 2) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 3) == rank) {
			this->addMoveToList(&moves, from, from - 3, { from - 1, from - 2 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 3) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 4) == rank) {
			this->addMoveToList(&moves, from, from - 4, { from - 1, from - 2, from - 3 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 4) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 5) == rank) {
			this->addMoveToList(&moves, from, from - 5, { from - 1, from - 2, from - 3, from - 4 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 5) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 6) == rank) {
			this->addMoveToList(&moves, from, from - 6, { from - 1, from - 2, from - 3, from - 4, from - 5 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 6) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 7) == rank) {
			this->addMoveToList(&moves, from, from - 7, { from - 1, from - 2, from - 3, from - 4, from - 5, from - 6 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 7) << " (not same rank)" << std::endl;
		}

		this->addMoveToListDiag(&moves, from, from +  9, {});
		this->addMoveToListDiag(&moves, from, from + 18, { from + 9 });
		this->addMoveToListDiag(&moves, from, from + 27, { from + 9, from + 18 });
		this->addMoveToListDiag(&moves, from, from + 36, { from + 9, from + 18, from + 27 });
		this->addMoveToListDiag(&moves, from, from + 45, { from + 9, from + 18, from + 27, from + 36 });
		this->addMoveToListDiag(&moves, from, from + 54, { from + 9, from + 18, from + 27, from + 36, from + 45 });
		this->addMoveToListDiag(&moves, from, from + 63, { from + 9, from + 18, from + 27, from + 36, from + 45, from + 54 });

		this->addMoveToListDiag(&moves, from, from -  9, {});
		this->addMoveToListDiag(&moves, from, from - 18, { from - 9 });
		this->addMoveToListDiag(&moves, from, from - 27, { from - 9, from - 18 });
		this->addMoveToListDiag(&moves, from, from - 36, { from - 9, from - 18, from - 27 });
		this->addMoveToListDiag(&moves, from, from - 45, { from - 9, from - 18, from - 27, from - 36 });
		this->addMoveToListDiag(&moves, from, from - 54, { from - 9, from - 18, from - 27, from - 36, from - 45 });
		this->addMoveToListDiag(&moves, from, from - 63, { from - 9, from - 18, from - 27, from - 36, from - 45, from - 54 });

		this->addMoveToListDiag(&moves, from, from +  7, {});
		this->addMoveToListDiag(&moves, from, from + 14, { from + 7 });
		this->addMoveToListDiag(&moves, from, from + 21, { from + 7, from + 14 });
		this->addMoveToListDiag(&moves, from, from + 28, { from + 7, from + 14, from + 21 });
		this->addMoveToListDiag(&moves, from, from + 35, { from + 7, from + 14, from + 21, from + 28 });
		this->addMoveToListDiag(&moves, from, from + 42, { from + 7, from + 14, from + 21, from + 28, from + 35 });
		this->addMoveToListDiag(&moves, from, from + 49, { from + 7, from + 14, from + 21, from + 28, from + 35, from + 42 });

		this->addMoveToListDiag(&moves, from, from -  7, {});
		this->addMoveToListDiag(&moves, from, from - 14, { from - 7 });
		this->addMoveToListDiag(&moves, from, from - 21, { from - 7, from - 14 });
		this->addMoveToListDiag(&moves, from, from - 28, { from - 7, from - 14, from - 21 });
		this->addMoveToListDiag(&moves, from, from - 35, { from - 7, from - 14, from - 21, from - 28 });
		this->addMoveToListDiag(&moves, from, from - 42, { from - 7, from - 14, from - 21, from - 28, from - 35 });
		this->addMoveToListDiag(&moves, from, from - 49, { from - 7, from - 14, from - 21, from - 28, from - 35, from - 42 });

		current_queens = current_queens & ~(1ULL << from);
	}
	

	// rooks
	unsigned long long current_rooks = this->rooks & pieces;
	std::cout << "info string [Board::getAllMoves] ROOKS ------------------------------" << std::endl;
	from = __builtin_ffsll(current_rooks) - 1;
	while (current_rooks > 0) {
		from = __builtin_ffsll(current_rooks) - 1;
		std::cout << "info string [Board::getAllMoves] rook at " << Lib::getCoordinatesFromBitnum(from) << std::endl;

		this->addMoveToList(&moves, from, from +  8, {});
		this->addMoveToList(&moves, from, from + 16, { from + 8 });
		this->addMoveToList(&moves, from, from + 24, { from + 8, from + 16 });
		this->addMoveToList(&moves, from, from + 32, { from + 8, from + 16, from + 24 });
		this->addMoveToList(&moves, from, from + 40, { from + 8, from + 16, from + 24, from + 32 });
		this->addMoveToList(&moves, from, from + 48, { from + 8, from + 16, from + 24, from + 32, from + 40 });
		this->addMoveToList(&moves, from, from + 56, { from + 8, from + 16, from + 24, from + 32, from + 40, from + 48 });

		this->addMoveToList(&moves, from, from -  8, {});
		this->addMoveToList(&moves, from, from - 16, { from - 8 });
		this->addMoveToList(&moves, from, from - 24, { from - 8, from - 16 });
		this->addMoveToList(&moves, from, from - 32, { from - 8, from - 16, from - 24 });
		this->addMoveToList(&moves, from, from - 40, { from - 8, from - 16, from - 24, from - 32 });
		this->addMoveToList(&moves, from, from - 48, { from - 8, from - 16, from - 24, from - 32, from - 40 });
		this->addMoveToList(&moves, from, from - 56, { from - 8, from - 16, from - 24, from - 32, from - 40, from - 48 });

		int rank = Lib::getRank(from);
		if (Lib::getRank(from + 1) == rank) {
			this->addMoveToList(&moves, from, from + 1, {});
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 1) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 2) == rank) {
			this->addMoveToList(&moves, from, from + 2, { from + 1 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 2) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 3) == rank) {
			this->addMoveToList(&moves, from, from + 3, { from + 1, from + 2 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 3) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 4) == rank) {
			this->addMoveToList(&moves, from, from + 4, { from + 1, from + 2, from + 3 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 4) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 5) == rank) {
			this->addMoveToList(&moves, from, from + 5, { from + 1, from + 2, from + 3, from + 4 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 5) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 6) == rank) {
			this->addMoveToList(&moves, from, from + 6, { from + 1, from + 2, from + 3, from + 4, from + 5 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 6) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from + 7) == rank) {
			this->addMoveToList(&moves, from, from + 7, { from + 1, from + 2, from + 3, from + 4, from + 5, from + 6 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from + 7) << " (not same rank)" << std::endl;
		}


		if (Lib::getRank(from - 1) == rank) {
			this->addMoveToList(&moves, from, from - 1, {});
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 1) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 2) == rank) {
			this->addMoveToList(&moves, from, from - 2, { from - 1 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 2) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 3) == rank) {
			this->addMoveToList(&moves, from, from - 3, { from - 1, from - 2 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 3) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 4) == rank) {
			this->addMoveToList(&moves, from, from - 4, { from - 1, from - 2, from - 3 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 4) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 5) == rank) {
			this->addMoveToList(&moves, from, from - 5, { from - 1, from - 2, from - 3, from - 4 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 5) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 6) == rank) {
			this->addMoveToList(&moves, from, from - 6, { from - 1, from - 2, from - 3, from - 4, from - 5 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 6) << " (not same rank)" << std::endl;
		}
		if (Lib::getRank(from - 7) == rank) {
			this->addMoveToList(&moves, from, from - 7, { from - 1, from - 2, from - 3, from - 4, from - 5, from - 6 });
		} else {
			std::cout << "info string [Board::getAllMoves] cannot move to " << Lib::getCoordinatesFromBitnum(from - 7) << " (not same rank)" << std::endl;
		}

		current_rooks = current_rooks & ~(1ULL << from);
	}


	// bishops
	unsigned long long current_bishops = this->bishops & pieces;
	std::cout << "info string [Board::getAllMoves] BISHOPS ------------------------------" << std::endl;
	from = __builtin_ffsll(current_bishops) - 1;
	while (current_bishops > 0) {
		from = __builtin_ffsll(current_bishops) - 1;
		std::cout << "info string [Board::getAllMoves] bishop at " << Lib::getCoordinatesFromBitnum(from) << std::endl;

		this->addMoveToListDiag(&moves, from, from +  9, {});
		this->addMoveToListDiag(&moves, from, from + 18, { from + 9 });
		this->addMoveToListDiag(&moves, from, from + 27, { from + 9, from + 18 });
		this->addMoveToListDiag(&moves, from, from + 36, { from + 9, from + 18, from + 27 });
		this->addMoveToListDiag(&moves, from, from + 45, { from + 9, from + 18, from + 27, from + 36 });
		this->addMoveToListDiag(&moves, from, from + 54, { from + 9, from + 18, from + 27, from + 36, from + 45 });
		this->addMoveToListDiag(&moves, from, from + 63, { from + 9, from + 18, from + 27, from + 36, from + 45, from + 54 });

		this->addMoveToListDiag(&moves, from, from -  9, {});
		this->addMoveToListDiag(&moves, from, from - 18, { from - 9 });
		this->addMoveToListDiag(&moves, from, from - 27, { from - 9, from - 18 });
		this->addMoveToListDiag(&moves, from, from - 36, { from - 9, from - 18, from - 27 });
		this->addMoveToListDiag(&moves, from, from - 45, { from - 9, from - 18, from - 27, from - 36 });
		this->addMoveToListDiag(&moves, from, from - 54, { from - 9, from - 18, from - 27, from - 36, from - 45 });
		this->addMoveToListDiag(&moves, from, from - 63, { from - 9, from - 18, from - 27, from - 36, from - 45, from - 54 });

		this->addMoveToListDiag(&moves, from, from +  7, {});
		this->addMoveToListDiag(&moves, from, from + 14, { from + 7 });
		this->addMoveToListDiag(&moves, from, from + 21, { from + 7, from + 14 });
		this->addMoveToListDiag(&moves, from, from + 28, { from + 7, from + 14, from + 21 });
		this->addMoveToListDiag(&moves, from, from + 35, { from + 7, from + 14, from + 21, from + 28 });
		this->addMoveToListDiag(&moves, from, from + 42, { from + 7, from + 14, from + 21, from + 28, from + 35 });
		this->addMoveToListDiag(&moves, from, from + 49, { from + 7, from + 14, from + 21, from + 28, from + 35, from + 42 });

		this->addMoveToListDiag(&moves, from, from -  7, {});
		this->addMoveToListDiag(&moves, from, from - 14, { from - 7 });
		this->addMoveToListDiag(&moves, from, from - 21, { from - 7, from - 14 });
		this->addMoveToListDiag(&moves, from, from - 28, { from - 7, from - 14, from - 21 });
		this->addMoveToListDiag(&moves, from, from - 35, { from - 7, from - 14, from - 21, from - 28 });
		this->addMoveToListDiag(&moves, from, from - 42, { from - 7, from - 14, from - 21, from - 28, from - 35 });
		this->addMoveToListDiag(&moves, from, from - 49, { from - 7, from - 14, from - 21, from - 28, from - 35, from - 42 });

		current_bishops = current_bishops & ~(1ULL << from);
	}


	// knights
	unsigned long long current_knights = this->knights & pieces;
	std::cout << "info string [Board::getAllMoves] KNIGHTS ------------------------------" << std::endl;
	while (current_knights > 0) {
		from = __builtin_ffsll(current_knights) - 1;
		std::cout << "info string [Board::getAllMoves] knight at " << Lib::getCoordinatesFromBitnum(from) << std::endl;

		this->addMoveToList(&moves, from, from -  6, {}, 2);
		this->addMoveToList(&moves, from, from - 10, {}, 2);
		this->addMoveToList(&moves, from, from - 15, {}, 2);
		this->addMoveToList(&moves, from, from - 17, {}, 2);
		this->addMoveToList(&moves, from, from +  6, {}, 2);
		this->addMoveToList(&moves, from, from + 10, {}, 2);
		this->addMoveToList(&moves, from, from + 15, {}, 2);
		this->addMoveToList(&moves, from, from + 17, {}, 2);

		current_knights = current_knights & ~(1ULL << from);
	}
	

	// pawns
	unsigned long long current_pawns = this->pawns & pieces;
	std::cout << "info string [Board::getAllMoves] PAWNS ------------------------------" << std::endl;
	while (current_pawns > 0) {
		from = __builtin_ffsll(current_pawns) - 1;
		std::cout << "info string [Board::getAllMoves] pawn at " << Lib::getCoordinatesFromBitnum(from) << std::endl;
		
		// pawn moves
		if (this->playerToMove == 'w') {
			this->addMoveToList(&moves, from, from + 8, {from + 8} );
			if (from <= Lib::getBitnumFromCoordinates("a2")) {
				this->addMoveToList(&moves, from, from + 16, {from + 8, from + 16} );
			}
		} else {
			this->addMoveToList(&moves, from, from - 8, {from - 8} );
			if (from >= Lib::getBitnumFromCoordinates("h7")) {
				this->addMoveToList(&moves, from, from - 16, {from - 8, from - 16} );
			}
		}
		
		// pawn beatings
		if (this->playerToMove == 'w') {
			// pawn beatings for white
			to = from + 7;
			if /* check for borders */(to % 8 != 7) {
				if ((this->blacks & (1ULL << to)) > 0) {
					std::cout << "info string [Board::getAllMoves] piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
					this->addMoveToList(&moves, from, to);
				} else {
					std::cout << "info string [Board::getAllMoves] no piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
				}
			}

			to = from + 9;
			if /* check for borders */(to % 8 != 7) {
				if ((this->blacks & (1ULL << to)) > 0) {
					std::cout << "info string [Board::getAllMoves] piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
					this->addMoveToList(&moves, from, to);
				} else {
					std::cout << "info string [Board::getAllMoves] no piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
				}
			}
		} else {
			// pawn beatings for black
			to = from - 7;
			if /* check for borders */(to % 8 != 7) {
				if ((this->blacks & (1ULL << to)) > 0) {
					std::cout << "info string [Board::getAllMoves] piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
					this->addMoveToList(&moves, from, to);
				} else {
					std::cout << "info string [Board::getAllMoves] no piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
				}
			}

			to = from - 9;
			if /* check for borders */(to % 8 != 7) {
				if ((this->blacks & (1ULL << to)) > 0) {
					std::cout << "info string [Board::getAllMoves] piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
					this->addMoveToList(&moves, from, to);
				} else {
					std::cout << "info string [Board::getAllMoves] no piece at " << Lib::getCoordinatesFromBitnum(to) << " can be beaten" << std::endl;
				}
			}
		}

		// TODO: pawn promotions

		// TODO: en passant moves
		
		current_pawns = current_pawns & ~(1ULL << from);
	}
	
	
	
	// TODO: different move lists for "captures, checks and checkmates, [en passant, promotions, castles|]"

	std::cout << "info string [Board::getAllMoves] ------------------------------" << std::endl;
	
	return moves;
}
