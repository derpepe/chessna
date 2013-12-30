#include "Board.h"

Board::Board()
{
	this->clear();
}

void Board::clear()
{
	this->blacks  = 0xffff000000000000;
	this->whites  = 0x000000000000ffff;
	
	this->kings   = 0x0800000000000008;
	this->queens  = 0x1000000000000010;
	this->rooks   = 0x8100000000000081;
	this->bishops = 0x2400000000000024;
	this->knights = 0x4200000000000042;
	this->pawns   = 0x00ff00000000ff00;
}

void Board::loadFen(std::string fen)
{
	std::istringstream fenstream(fen);
	std::vector<std::string> fields{std::istream_iterator<std::string>{fenstream}, std::istream_iterator<std::string>{}};
	// fields = Figurenstellung " " Am Zug " " Rochade " " en passant " " Halbzüge " " Zugnummer
	
	// TODO: load fen
}

std::string Board::getDump()
{
	std::string prefix = "info string ";
	std::ostringstream result;
	unsigned long long figures = (this->whites | this->blacks);
	unsigned long long all = this->rooks | this->knights | this->bishops | this->queens | this->kings | this->pawns;
	if (all == figures)
	{
		result << prefix << "board information is consistent." << std::endl;
	}
	else
	{
		result << prefix << "board information is INCONSISTENT." << std::endl;
	}
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
			}
			else
			{
				f = ".";
			}
			result << f;
		}
		result << std::endl;
	}
	return result.str();
}
