#include "Comm.h"
#include <iostream>
#include <stdexcept>

Comm::Comm() {
  uciOutputCallback = [](const std::string &message) {
    std::cerr << "Warning: uciOutput callback not set. Message: " << message
              << std::endl;
  };
  engineGoCallback = [](GoParams) {
    std::cerr << "Warning: engineGo callback not set" << std::endl;
  };
  engineStopCallback = []() {
    std::cerr << "Warning: engineStop callback not set" << std::endl;
  };
  engineSetPositionCallback = [](const std::string &) {
    std::cerr << "Warning: engineSetPosition callback not set" << std::endl;
  };
  engineExecuteMoveCallback = [](const std::string &) {
    std::cerr << "Warning: engineExecuteMove callback not set" << std::endl;
  };
  engineDebugCallback = []() {
    std::cerr << "Warning: engineDebug callback not set" << std::endl;
  };
  engineListMovesCallback = []() {
    std::cerr << "Warning: engineListMoves callback not set" << std::endl;
  };
  enginePerftCallback = [](int) {
    std::cerr << "Warning: enginePerft callback not set" << std::endl;
  };
  enginePerftDivideCallback = [](int) {
    std::cerr << "Warning: enginePerftDivide callback not set" << std::endl;
  };
  enginePerftNodesCallback = [](int) {
    std::cerr << "Warning: enginePerftNodes callback not set" << std::endl;
    return 0ULL;
  };
}

void Comm::registerUciOutputCallback(
    std::function<void(std::string)> uciOutputCallback) {
  this->uciOutputCallback = uciOutputCallback;
}
void Comm::uciOutput(std::string message) {
  if (uciOutputCallback) {
    uciOutputCallback(message);
  } else {
    throw std::runtime_error("uciOutput callback not set");
  }
}

void Comm::registerEngineGoCallback(std::function<void(GoParams)> engineGoCallback) {
  this->engineGoCallback = engineGoCallback;
}
void Comm::engineGo(GoParams params) {
  if (engineGoCallback) {
    engineGoCallback(params);
  } else {
    throw std::runtime_error("engineGo callback not set");
  }
}

void Comm::registerEngineStopCallback(
    std::function<void()> engineStopCallback) {
  this->engineStopCallback = engineStopCallback;
}
void Comm::engineStop() {
  if (engineStopCallback) {
    engineStopCallback();
  } else {
    throw std::runtime_error("engineStop callback not set");
  }
}

void Comm::registerEngineSetPositionCallback(
    std::function<void(std::string)> engineSetPositionCallback) {
  this->engineSetPositionCallback = engineSetPositionCallback;
}
void Comm::engineSetPosition(std::string fen) {
  if (engineSetPositionCallback) {
    engineSetPositionCallback(fen);
  } else {
    throw std::runtime_error("engineSetPosition callback not set");
  }
}

void Comm::registerEngineExecuteMoveCallback(
    std::function<void(std::string)> engineExecuteMoveCallback) {
  this->engineExecuteMoveCallback = engineExecuteMoveCallback;
}
void Comm::engineExecuteMove(std::string move) {
  if (engineExecuteMoveCallback) {
    engineExecuteMoveCallback(move);
  } else {
    throw std::runtime_error("engineExecuteMove callback not set");
  }
}

void Comm::registerEngineDebugCallback(
    std::function<void()> engineDebugCallback) {
  this->engineDebugCallback = engineDebugCallback;
}
void Comm::engineDebug() {
  if (engineDebugCallback) {
    engineDebugCallback();
  } else {
    throw std::runtime_error("engineDebug callback not set");
  }
}

void Comm::registerEngineListMovesCallback(
    std::function<void()> engineListMovesCallback) {
  this->engineListMovesCallback = engineListMovesCallback;
}
void Comm::engineListMoves() {
  if (engineListMovesCallback) {
    engineListMovesCallback();
  } else {
    throw std::runtime_error("engineListMoves callback not set");
  }
}

void Comm::registerEnginePerftCallback(
    std::function<void(int)> enginePerftCallback) {
  this->enginePerftCallback = enginePerftCallback;
}
void Comm::enginePerft(int depth) {
  if (enginePerftCallback) {
    enginePerftCallback(depth);
  } else {
    throw std::runtime_error("enginePerft callback not set");
  }
}

void Comm::registerEnginePerftDivideCallback(
    std::function<void(int)> enginePerftDivideCallback) {
  this->enginePerftDivideCallback = enginePerftDivideCallback;
}
void Comm::enginePerftDivide(int depth) {
  if (enginePerftDivideCallback) {
    enginePerftDivideCallback(depth);
  } else {
    throw std::runtime_error("enginePerftDivide callback not set");
  }
}

void Comm::registerEnginePerftNodesCallback(
    std::function<unsigned long long(int)> enginePerftNodesCallback) {
  this->enginePerftNodesCallback = enginePerftNodesCallback;
}
unsigned long long Comm::enginePerftNodes(int depth) {
  if (enginePerftNodesCallback) {
    return enginePerftNodesCallback(depth);
  }
  throw std::runtime_error("enginePerftNodes callback not set");
}
