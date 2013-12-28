#include "Uci.h"


Uci::Uci(Comm *comm)
{
	this->comm = comm;

	this->comm->registerUciCallback( [this](std::string message) { this->commCallback(message); } );
}

void Uci::commCallback(std::string message)
{
	this->sendString(message);
}

void Uci::run()
{
	std::string input = "";
	
	while(true)
	{
		std::getline(std::cin,input);
		std::cout << "echo: \"" << input << "\"" << std::endl;
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
		this->comm->engine("go");
	}
	else if (command.compare("stop") == 0)
	{
		this->sendBestmove("e2e4", "");
	}
	else if (command.compare("position") == 0)
	{
		std::vector<std::string>::iterator tokenIterator = parameters.begin();
		++tokenIterator; // skip command
		std::string position = *tokenIterator; // get second word
		
		std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		if (position.compare("fen") == 0)
		{
			++tokenIterator; // skip 'fen'
			fen = *tokenIterator;
		}
		++tokenIterator; // skip fenstring or 'startpos'
		
		std::vector<std::string> moves;
		for ( ; tokenIterator != parameters.end(); ++tokenIterator)
		{
			moves.push_back(*tokenIterator);
		}
		
		// this->comm->engineSetposition(fen, parameters);
		std::cout << "fen: " << fen << std::endl;
		std::cout << "moves: ";
		for( std::vector<std::string>::iterator i = moves.begin(); i != moves.end(); ++i)
		{
		    std::cout << *i << ' ';
		}
		std::cout << std::endl;
		
			/*
		
		* position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
			set up the position described in fenstring on the internal board and
			play the moves on the internal chess board.
			if the game was played  from the start position the string "startpos" will be sent
			Note: no "new" command is needed. However, if this position is from a different game than
			the last position sent to the engine, the GUI should have sent a "ucinewgame" inbetween.
		
		
			*/
	}
	
	// following internal commands
	else if (command.compare("fentest") == 0)
	{
		this->comm->engine("fentest");
	}
	else
	{
		// ignore unknown command
	}
}


void Uci::sendString(std::string message)
{
	std::cout << message << std::endl;
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