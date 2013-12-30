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
	std::cout << "info string [Engine:setPosition] settings position to '" << position << "'" << std::endl;
	this->board->loadFen(position);
	// TODO
}


void Engine::executeMove(std::string move)
{
	std::cout << "info string [Engine::executeMove] execute move '" << move << "'" << std::endl;
	this->board->executeMove(move);
}

void Engine::debug()
{
	std::string boardDump = this->board->getDump();
	this->comm->uciOutput(boardDump);
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
	std::ostringstream output;
	output << "bestmove e2e4" << std::endl;
	this->comm->uciOutput(output.str()); // TODO
}
