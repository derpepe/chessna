#ifndef UCI_H
#define UCI_H

#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>
#include <string>
#include "Comm.h"
#include "Lib.h"

class Uci
{
public:
	Uci(Comm *);

	void run();
	void parse(std::string);
	
	void sendString(std::string);
	void sendId(std::string, std::string);
	void sendUciok();
	void sendReadyok();
	void sendBestmove(std::string, std::string);

private:
	Comm *comm;
};

#endif