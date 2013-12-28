#ifndef UCI_H
#define UCI_H

#include <iostream>
class Engine; // forward declaration for the compiler


class Uci
{
public:
	void commLoop();
	void registerEngine(Engine *engine);

	void parse(std::string);
	
	void sendString(std::string);
	void sendId(std::string, std::string);
	void sendUciok();
	void sendReadyok();

private:
	Engine *engine;
};

#endif