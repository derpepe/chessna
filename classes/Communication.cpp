#include "Communication.h"
#include <iostream>

Communication::Communication
{
	this->uci = new Uci();
}

void Communication::mainLoop()
{
	std::string input = "";
	
	while(true)
	{
		std::cin >> input;
		this->uci->parse(input);
	}
}