#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Comm.h"
#include <vector>
#include <string>

class Engine
{
public:
	Engine(Comm* comm);
	void commCallback(std::string);
	
	void run();
	
	// commands sent via UCI
	void go();
	void setPosition(std::string);
	void executeMoves(std::vector<std::string>);
	
private:
	Board *board;
	Comm *comm;
};

#endif