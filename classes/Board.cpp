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
	std::cout << "Am Zug: " << fields[1];
}
