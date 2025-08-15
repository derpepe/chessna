#define DEBUG

#include "Board.h"
#include "MoveGenerator.h"
#include <iostream>

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
	// std::cout << "info string [Board::executeMove] moves figure from " << from << " to " << to << std::endl;
#endif
	
        // check if a figure gets captured (for potential reset of halfmoves-counter)
        bool isPawn = (this->pawns & (1ULL << from)) != 0;
        bool enPassantCapture = false;
        int captureSquare = to;

        if (isPawn && this->enPassant != "-")
        {
                int ep_square = Lib::getBitnumFromCoordinates(this->enPassant);
                if (to == ep_square && (abs(to - from) == 7 || abs(to - from) == 9))
                {
                        enPassantCapture = true;
                        captureSquare = (this->playerToMove == 'w') ? to - 8 : to + 8;
                }
        }

        bool figureCaptured = enPassantCapture || (((1ULL << to) & (this->whites | this->blacks)) != 0);

        if (figureCaptured) {
                unsigned long long capture_mask = ~(1ULL << captureSquare);
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

std::vector<std::string> Board::getAllMoves()
{
	MoveGenerator moveGenerator;
	return moveGenerator.getAllMoves(*this);
}

