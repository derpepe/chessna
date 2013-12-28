#include <thread>
#include "Board.h"
#include "Uci.h"


int main(int argc, char** args)
{
	Uci *uci = new Uci();

	// initialize asynchronous communication
	std::thread iothread( [uci] { uci->mainLoop(); } );

	// start main program here
	Board *board = new Board();
	while(true) {
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		uci->engineSays("hello");
	}
	
	iothread.join();
	return 0;
}
