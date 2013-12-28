#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Comm.h"

class Engine
{
public:
	Engine(Comm* comm);
	void commCallback(std::string);
	
	void run();
	
	// commands sent via UCI
	void go();
	
private:
	Board *board;
	Comm *comm;
};

#endif