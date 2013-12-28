#include <thread>
#include "Uci.h"
#include "Engine.h"
#include "Comm.h"


int main(int argc, char** args)
{
	Comm *comm = new Comm();
	
	Uci *uci = new Uci(comm);
	Engine *engine = new Engine(comm);

	// initialize asynchronous communication
	std::thread uci_thread( [uci] { uci->run(); } );
	std::thread engine_thread( [engine] { engine->run(); } );

	// wait for both threads
	uci_thread.join();
	engine_thread.join();
	return 0;
}
