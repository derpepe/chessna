#include <iostream>
#include "Board.h"

int main(int argc, char** args)
{
	Board *board = new Board();
	board->clear();
	std::cout << sizeof(board->kings) << " (should be 8 byte)";
	return 0;
}
