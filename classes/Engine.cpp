#include "Engine.h"

Engine::Engine(Uci * uci)
{
	this->uci = uci;
	this->board = new Board();
}

void Engine::run()
{
	while(true)
	{
		this->uci->sendString("hello");
	}
}