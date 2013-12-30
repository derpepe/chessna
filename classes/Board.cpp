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
	if ((this->rooks & (1ULL << to)) != 0)
	{
		// invalidate casteling
		switch(from)
		{
			case 0:
				std::cout << "info string [Board::executeMove] White rook h1 moved. Invlaidate casteling." << std::endl;
				casteling_K = false;
				break;
			case 7:
				std::cout << "info string [Board::executeMove] White rook a1 moved. Invlaidate casteling." << std::endl;
				casteling_Q = false;
				break;
			case 56:
				std::cout << "info string [Board::executeMove] Black rook h8 moved. Invlaidate casteling." << std::endl;
				casteling_k = false;
				break;
			case 63:
				std::cout << "info string [Board::executeMove] Black rook a8 moved. Invlaidate casteling." << std::endl;
				casteling_q = false;
				break;
			
		}
	}
	if (to == 0)
	{
		std::cout << "info string [Board::executeMove] To field is h1. Invlaidate casteling." << std::endl;
		casteling_K = false;
	}
	else if (to == 7)
	{
		std::cout << "info string [Board::executeMove] To field is a1. Invlaidate casteling." << std::endl;
		casteling_Q = false;
	}
	else if (to == 56)
	{
		std::cout << "info string [Board::executeMove] To field is h8. Invlaidate casteling." << std::endl;
		casteling_k = false;
	}
	else if (to == 63)
	{
		std::cout << "info string [Board::executeMove] To field is a8. Invlaidate casteling." << std::endl;
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
	
	// TODO: en passant
	
	
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
		// TODO: potential reset of halfmoves-counter
		
		// update player
		this->playerToMove = this->playerToMove == 'w' ? 'b' : 'w';
	
		// update moves counter
		this->halfmoves++;
		if (this->playerToMove == 'w') // increment after move of black
		{
			this->currentMove++;
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
				f = "·";
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