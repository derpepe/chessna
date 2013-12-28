#include <iostream>
#include "Board.h"
#include "Uci.h"

int main(int argc, char** args)
{
	Board *board = new Board();
	Uci *uci = new Uci();
	
	uci->sendId("name", "CHESSna 2 Version 0.01 alpha");
	uci->sendId("author", "Peter Schneider");
	uci->sendUciok();

	return 0;
}
