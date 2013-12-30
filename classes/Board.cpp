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


void Board::executeMove(std::string move)
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
	unsigned long long figures = (this->whites | this->blacks);
	int emptycount = 0;
	for (int y = 0; y < 8; y++) {
		result << prefix;
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
				f = ".";
				emptycount++;
			}
			result << f;
		}
		if (emptycount > 0)
		{
			fen << emptycount;
			emptycount = 0;
		}
		if (y < 7) fen << '/';
		result << std::endl;
	}
	if (emptycount > 0)
	{
		fen << emptycount;
		emptycount = 0;
	}
	result << prefix << std::endl;

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