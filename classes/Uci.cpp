#include "Uci.h"

void Uci::commLoop()
{
	std::string input = "";
	
	while(true)
	{
		std::cin >> input;
		this->parse(input);
	}
}

void Uci::registerEngine(Engine *engine)
{
	this->engine = engine;
}


void Uci::parse(std::string command)
{
	if (command.compare("uci") == 0)
	{
		this->sendId("name", "CHESSna 2 Version 0.01 alpha");
		this->sendId("author", "Peter Schneider");
		this->sendUciok();
	}
	else if (command.compare("isready") == 0)
	{
		while (this->engine == NULL) {}
		this->sendReadyok();
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