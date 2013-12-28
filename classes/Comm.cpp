#include "Comm.h"


void Comm::registerUciCallback(std::function<void(std::string)> uciCallback)
{
	this->uciCallback = uciCallback;
}

void Comm::registerEngineCallback(std::function<void(std::string)> engineCallback)
{
	this->engineCallback = engineCallback;
}


void Comm::uci(std::string message)
{
	this->uciCallback(message);
}

void Comm::engine(std::string message)
{
	this->engineCallback(message);
}
