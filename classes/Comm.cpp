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

void Comm::registerEngineListMovesCallback(std::function<void()> engineListMovesCallback) { this->engineListMovesCallback = engineListMovesCallback; }
void Comm::engineListMoves() { this->engineListMovesCallback(); }

void Comm::registerEnginePerftCallback(std::function<void(int)> enginePerftCallback) { this->enginePerftCallback = enginePerftCallback; }
void Comm::enginePerft(int depth) { this->enginePerftCallback(depth); }

void Comm::registerEnginePerftDivideCallback(std::function<void(int)> enginePerftDivideCallback) { this->enginePerftDivideCallback = enginePerftDivideCallback; }
void Comm::enginePerftDivide(int depth) { this->enginePerftDivideCallback(depth); }

void Comm::registerEnginePerftNodesCallback(std::function<unsigned long long(int)> enginePerftNodesCallback) { this->enginePerftNodesCallback = enginePerftNodesCallback; }
unsigned long long Comm::enginePerftNodes(int depth) { return this->enginePerftNodesCallback(depth); }
