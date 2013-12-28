#ifndef COMM_H
#define COMM_H

#include <iostream>
#include <string>
#include <vector>

class Comm
{
public:
	void registerUciOutputCallback(std::function<void(std::string)>);
	void uciOutput(std::string);

	void registerEngineGoCallback(std::function<void()>);
	void engineGo();	
	void registerEngineSetPositionCallback(std::function<void(std::string)>);
	void engineSetPosition(std::string);
	void registerEngineExecuteMovesCallback(std::function<void(std::vector<std::string>)>);
	void engineExecuteMoves(std::vector<std::string>);
	
private:
	std::function<void(std::string)> uciOutputCallback;

	std::function<void()> engineGoCallback;
	std::function<void(std::string)> engineSetPositionCallback;
	std::function<void(std::vector<std::string>)> engineExecuteMovesCallback;
};
#endif