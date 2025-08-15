#ifndef COMM_H
#define COMM_H

#include <iostream>
#include <string>
#include <functional>

class Comm
{
public:
	void registerUciOutputCallback(std::function<void(std::string)>);
	void uciOutput(std::string);

	void registerEngineGoCallback(std::function<void()>);
	void engineGo();
	void registerEngineSetPositionCallback(std::function<void(std::string)>);
	void engineSetPosition(std::string);
	void registerEngineExecuteMoveCallback(std::function<void(std::string)>);
	void engineExecuteMove(std::string);
	void registerEngineDebugCallback(std::function<void()>);
	void engineDebug();
	void registerEnginePerftCallback(std::function<void(int)>);
	void enginePerft(int);
	void registerEnginePerftDivideCallback(std::function<void(int)>);
	void enginePerftDivide(int);
	
private:
	std::function<void(std::string)> uciOutputCallback;

	std::function<void()> engineGoCallback;
	std::function<void(std::string)> engineSetPositionCallback;
	std::function<void(std::string)> engineExecuteMoveCallback;
	std::function<void()> engineDebugCallback;
	std::function<void(int)> enginePerftCallback;
	std::function<void(int)> enginePerftDivideCallback;
};
#endif
