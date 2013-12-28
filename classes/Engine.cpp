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
		for (int i = 0; i < 100000; i++) {}
		this->comm->uci("hello");
	}
}

void Engine::go()
{
	this->comm->uci("bestmove e2e4");
}