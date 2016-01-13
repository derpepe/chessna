#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Comm.h"
#include <vector>
#include <string>
#include <random>

class Engine
{
public:
	Engine(Comm* comm);	
	void run();
	
	// commands sent via UCI
	void go();
	void setPosition(std::string);
	void executeMove(std::string);
	void debug();
	
private:
	Board *board;
	Comm *comm;
};

#endif