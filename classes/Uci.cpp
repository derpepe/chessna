#include "Uci.h"


Uci::Uci(Comm *comm)
{
	this->comm = comm;

	this->comm->registerUciOutputCallback( [this](std::string message) { this->sendString(message); } );
}

void Uci::run()
{
	std::string input = "";
	
	while(true)
	{
		std::getline(std::cin,input);
		this->parse(input);
	}
}

void Uci::parse(std::string p_parameters)
{
	std::vector<std::string> parameters = Lib::split(p_parameters, ' ');
	if (parameters.size() == 0)
	{
		return;
	}
	
	// parse command
	std::string command = parameters[0];
	
	if (command.compare("uci") == 0)
	{
		this->sendId("name", "CHESSna 2 Version 0.01 alpha");
		this->sendId("author", "Peter Schneider");
		this->sendUciok();
	}
	else if (command.compare("isready") == 0)
	{
		this->sendReadyok();
	}
	else if (command.compare("go") == 0)
	{
		this->comm->engineGo();
	}
	else if (command.compare("quit") == 0)
	{
		//this->comm->engineStop();
		std::exit(0);
	}
	else if (command.compare("stop") == 0)
	{
		this->sendBestmove("e2e4", ""); // TODO: get best move from engine
	}
	else if (command.compare("position") == 0)
	{
		std::vector<std::string>::iterator tokenIterator = parameters.begin();
		++tokenIterator; // skip command
		std::string position = *tokenIterator; // get second word (startpos | fen)
		
		std::string fen = Fen::startPos;
		if (position.compare("fen") == 0)
		{
			++tokenIterator; // skip 'fen'
			std::ostringstream f;
			f << *tokenIterator; // position
			++tokenIterator;
			f << " " << *tokenIterator; // player to move
			++tokenIterator;
			f << " " << *tokenIterator; // casteling
			++tokenIterator;
			f << " " << *tokenIterator; // en passant
			++tokenIterator;
			f << " " << *tokenIterator; // halfmoves
			++tokenIterator;
			f << " " << *tokenIterator; // currentMove
			fen = f.str();
		}
		++tokenIterator; // skip last keyowrd ('startpos' or currentMove)
	
		this->comm->engineSetPosition(fen);

		if (tokenIterator != parameters.end()) ++tokenIterator; // skip 'moves'
		for ( ; tokenIterator != parameters.end(); ++tokenIterator)
		{
			this->comm->engineExecuteMove(*tokenIterator);
		}
	}
	
	// custom commands
	else if (command.compare("help") == 0)
	{
		std::ostringstream help;
		help << "info string CHESSna help for debugging console" << std::endl;
		help << "info string" << std::endl;
		help << "info string  board  Displays debug output." << std::endl;
		help << "info string  perft <depth>  Performance test of the move generation." << std::endl;
		this->sendString(help.str());
	}
	else if (command.compare("board") == 0)
	{
		this->comm->engineDebug();
	}
	else if (command.compare("perft") == 0)
	{
		int depth = std::stoi(parameters[1]);
		this->comm->enginePerft(depth);
	}
	else
	{
		// ignore unknown command
	}
}


void Uci::sendString(std::string message)
{
	std::cout << message;
}

void Uci::sendId(std::string key, std::string value)
{
	std::cout << "id " << key << " " << value << std::endl;
}

void Uci::sendUciok()
{
	std::cout << "uciok" << std::endl;
}

void Uci::sendReadyok()
{
	std::cout << "readyok" << std::endl;
}

void Uci::sendBestmove(std::string move, std::string ponder)
{
	std::cout << "bestmove " << move;
	if (ponder.compare("") != 0)
	{
		std::cout << " ponder " << ponder;
	}
	std::cout << std::endl;
}
