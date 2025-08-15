#include "Engine.h"


Engine::Engine(Comm *comm)
{
	this->comm = comm;
	this->comm->registerEngineGoCallback( [this] { this->go(); } );
	this->comm->registerEngineSetPositionCallback( [this](std::string position) { this->setPosition(position); } );
	this->comm->registerEngineExecuteMoveCallback( [this](std::string move) { this->executeMove(move); } );
	this->comm->registerEngineDebugCallback( [this] { this->debug(); } );
	this->comm->registerEnginePerftCallback( [this](int depth) { this->perft(depth); } );
	this->comm->registerEnginePerftDivideCallback( [this](int depth) { this->perftDivide(depth); } );
	
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
	this->comm->uciOutput(this->board->getDump());
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
	std::cout << "info string [Engine::go] let's go!" << std::endl;
	
	// TODO: invoke move generator
	std::vector<std::string> possibleMoves = this->board->getAllMoves();
	std::cout << "info string [Engine::go] found " << possibleMoves.size() << " moves" << std::endl;
	
	// TODO: decide which move is good
    std::random_device seed;
    std::mt19937 engine(seed());
    std::uniform_int_distribution<int> choose(0 , possibleMoves.size() - 1);
    std::string bestMove = possibleMoves[choose(engine)];

	std::ostringstream output;
	output << "bestmove " << bestMove << std::endl;
	this->comm->uciOutput(output.str()); // TODO
	// TODO: use UCI sendBestmove()?
}

void Engine::perft(int depth)
{
	std::cout << "info string [Engine::perft] starting perft(" << depth << ")" << std::endl;
	unsigned long long nodes = this->board->perft(depth);
	std::ostringstream output;
	output << "info string nodes " << nodes << std::endl;
	this->comm->uciOutput(output.str());
}

void Engine::perftDivide(int depth)
{
	std::cout << "info string [Engine::perftDivide] starting perftDivide(" << depth << ")" << std::endl;
	this->board->perftDivide(depth);
}
