#include "Board.h"

Board::Board()
{
	this->clear();
}

void Board::clear()
{
	blacks = 0;
	whites = 0;
	
	kings = 0;
	queens = 0;
	rooks = 0;
	bishops = 0;
	knights = 0;
	pawns = 0;
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
	std::string result = "";
	long long figures = this->whites | this->blacks;
	for (int x=0; x<=8; x++) {
		for (int y=0; y<=8; y++) {
			char f = '?';
			int bit = y*8 + x;
			long long position = 1 << bit;
			if (position & figures) {
				if (position & kings) f = 'W';
				if (position & queens) f = 'Q';
				if (position & rooks) f = 'R';
				if (position & knights) f = 'K';
				if (position & bishops) f = 'B';
				if (position & pawns) f = 'P';
				if (position & blacks) f = tolower(f);
			}
			else
			{
				f = '.';
			}
			result.append(f);
		}
		result.append(std::endl);
	}
	return result;
}
