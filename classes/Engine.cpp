#include "Engine.h"


Engine::Engine(Comm *comm)
{
	this->comm = comm;
	this->comm->registerEngineGoCallback( [this] { this->go(); } );
	this->comm->registerEngineSetPositionCallback( [this](std::string position) { this->setPosition(position); } );
	this->comm->registerEngineExecuteMoveCallback( [this](std::string move) { this->executeMove(move); } );
	this->comm->registerEngineDebugCallback( [this] { this->debug(); } );
	
	this->board = new Board();
}

void Engine::setPosition(std::string position)
{
	// TODO
}

void Engine::executeMove(std::string move)
{
	// TODO
}

void Engine::debug()
{
	this->comm->uciOutput("debug output goes here"); // TODO
}

void Engine::run()
{
	while(true)
	{
		// main computation should go here
	}
}

void Engine::go()
{
	this->comm->uciOutput("bestmove e2e4"); // TODO
}