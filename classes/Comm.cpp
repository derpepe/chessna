#include "Comm.h"


void Comm::registerUciOutputCallback(std::function<void(std::string)> uciOutputCallback) { this->uciOutputCallback = uciOutputCallback; }
void Comm::uciOutput(std::string message) { this->uciOutputCallback(message); }


void Comm::registerEngineGoCallback(std::function<void()> engineGoCallback) { this->engineGoCallback = engineGoCallback; }
void Comm::engineGo() { this->engineGoCallback(); }

void Comm::registerEngineSetPositionCallback(std::function<void(std::string)> engineSetPositionCallback) { this->engineSetPositionCallback = engineSetPositionCallback; }
void Comm::engineSetPosition(std::string fen) { this->engineSetPositionCallback(fen); }

void Comm::registerEngineExecuteMovesCallback(std::function<void(std::vector<std::string>)> engineExecuteMovesCallback) { this->engineExecuteMovesCallback = engineExecuteMovesCallback; }
void Comm::engineExecuteMoves(std::vector<std::string> moves) { this->engineExecuteMovesCallback(moves); }
