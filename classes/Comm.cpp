#include "Comm.h"


void Comm::registerUciOutputCallback(std::function<void(std::string)> uciOutputCallback) { this->uciOutputCallback = uciOutputCallback; }
void Comm::uciOutput(std::string message) { this->uciOutputCallback(message); }


void Comm::registerEngineGoCallback(std::function<void()> engineGoCallback) { this->engineGoCallback = engineGoCallback; }
void Comm::engineGo() { this->engineGoCallback(); }

void Comm::registerEngineSetPositionCallback(std::function<void(std::string)> engineSetPositionCallback) { this->engineSetPositionCallback = engineSetPositionCallback; }
void Comm::engineSetPosition(std::string fen) { this->engineSetPositionCallback(fen); }

void Comm::registerEngineExecuteMoveCallback(std::function<void(std::string)> engineExecuteMoveCallback) { this->engineExecuteMoveCallback = engineExecuteMoveCallback; }
void Comm::engineExecuteMove(std::string move) { this->engineExecuteMoveCallback(move); }

void Comm::registerEngineDebugCallback(std::function<void()> engineDebugCallback) { this->engineDebugCallback = engineDebugCallback; }
void Comm::engineDebug() { this->engineDebugCallback(); }
