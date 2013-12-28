#ifndef COMM_H
#define COMM_H

#include <iostream>

class Comm
{
public:
	void registerUciCallback(std::function<void(std::string)>);
	void registerEngineCallback(std::function<void(std::string)>);
	
	void uci(std::string);
	void engine(std::string);
	
private:
	std::function<void(std::string)> engineCallback;
	std::function<void(std::string)> uciCallback;
};
#endif