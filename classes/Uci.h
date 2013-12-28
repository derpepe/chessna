#ifndef UCI_H
#define UCI_H

#include <iostream>
#include "Comm.h"

class Uci
{
public:
	Uci(Comm *);
	void commCallback(std::string);

	void run();

	void parse(std::string);
	
	
	void sendString(std::string);
	void sendId(std::string, std::string);
	void sendUciok();
	void sendReadyok();

private:
	Comm *comm;
};

#endif