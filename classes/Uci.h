#ifndef UCI_H
#define UCI_H

#include <iostream>

class Uci
{
public:
	void parse(std::string);
	
	void sendId(std::string, std::string);
	void sendUciok();
	void sendReadyok();
};

#endif