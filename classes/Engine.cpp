#include "Engine.h"


Engine::Engine(Comm *comm)
{
	this->comm = comm;
	this->comm->registerEngineCallback( [this](std::string message) { this->commCallback(message); } );
	
	this->board = new Board();
}

void Engine::commCallback(std::string message)
{
	if (message.compare("go") == 0)
	{
		this->go();
	}
}

void Engine::run()
{
	while(true)
	{
		// compute whatever
	}
}

void Engine::go()
{
	this->comm->uci("bestmove e2e4");
}