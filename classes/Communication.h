#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "Uci.h"

class Communication
{
public:
	Communication();

	void mainLoop();

private:
	Uci *uci;
};

#endif