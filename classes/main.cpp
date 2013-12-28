#include <thread>
#include "Board.h"
#include "Uci.h"


int main(int argc, char** args)
{
	Uci *uci = new Uci();

	// initialize asynchronous communication
	std::thread comm( [uci] { uci->commLoop(); } );

	// start main program here
	Board *board = new Board();
	uci->sendString("hello");
	
	comm.join();
	return 0;
}
