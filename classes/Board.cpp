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
