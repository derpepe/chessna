#include <thread>
#include "Engine.h"
#include "Uci.h"


int main(int argc, char** args)
{
	Uci *uci = new Uci();

	// initialize asynchronous communication
	std::thread comm( [uci] { uci->commLoop(); } );

	// start main program here
	Engine *engine = new Engine(uci);
	engine->run();
	
	comm.join();
	return 0;
}
