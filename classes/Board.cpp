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
	std::istringstream fields(fen);
	std::vector<std::string> tokens{std::istream_iterator<std::string>{fields}, std::istream_iterator<std::string>{}};
	// tokens = Figurenstellung " " Am Zug " " Rochade " " en passant " " Halbzüge " " Zugnummer
	std::cout << tokens[0];
}
