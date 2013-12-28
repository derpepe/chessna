#include "Engine.h"


Engine::Engine(Comm *comm)
{
	this->comm = comm;
	this->comm->registerEngineGoCallback( [this] { this->go(); } );
	this->comm->registerEngineSetPositionCallback( [this](std::string position) { this->setPosition(position); } );
	this->comm->registerEngineExecuteMovesCallback( [this](std::vector<std::string> moves) { this->executeMoves(moves); } );
	
	this->board = new Board();
}

void Engine::setPosition(std::string position)
{
	//TODO
}

void Engine::executeMoves(std::vector<std::string> moves)
{
	//TODO
}

void Engine::commCallback(std::string message)
{
	if (message.compare("go") == 0)
	{
		this->go();
	}
	else
	{
		this->board->loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
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
	this->comm->uciOutput("bestmove e2e4");
}