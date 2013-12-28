#include "Engine.h"


Engine::Engine(Uci *uci)
{
	this->uci = uci;
	this->board = new Board();

	this->uci->registerEngine(this);
}

void Engine::run()
{
	while(true)
	{
		//this->uci->sendString("hello");
	}
}