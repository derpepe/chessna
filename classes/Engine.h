#ifndef ENGINE_H
#define ENGINE_H

#include "Board.h"
#include "Uci.h"

class Engine
{
public:
	Engine(Uci* uci);
	void run();
	
private:
	Board *board;
	Uci *uci;
};

#endif