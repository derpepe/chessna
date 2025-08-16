#ifndef COMM_H
#define COMM_H

#include <functional>
#include <iostream>
#include <string>

struct GoParams {
  int movetime = 0;
  int wtime = 0;
  int btime = 0;
  int winc = 0;
  int binc = 0;
};

class Comm {
public:
  Comm();
  void registerUciOutputCallback(std::function<void(std::string)>);
  void uciOutput(std::string);

  void registerEngineGoCallback(std::function<void(GoParams)>);
  void engineGo(GoParams);
  void registerEngineStopCallback(std::function<void()>);
  void engineStop();
  void registerEngineSetPositionCallback(std::function<void(std::string)>);
  void engineSetPosition(std::string);
  void registerEngineExecuteMoveCallback(std::function<void(std::string)>);
  void engineExecuteMove(std::string);
  void registerEngineDebugCallback(std::function<void()>);
  void engineDebug();
  void registerEngineListMovesCallback(std::function<void()>);
  void engineListMoves();
  void registerEngineEvaluateCallback(std::function<void()>);
  void engineEvaluate();
  void registerEnginePerftCallback(std::function<void(int)>);
  void enginePerft(int);
  void registerEnginePerftDivideCallback(std::function<void(int)>);
  void enginePerftDivide(int);
  void registerEnginePerftNodesCallback(std::function<unsigned long long(int)>);
  unsigned long long enginePerftNodes(int);

private:
  std::function<void(std::string)> uciOutputCallback;

  std::function<void(GoParams)> engineGoCallback;
  std::function<void()> engineStopCallback;
  std::function<void(std::string)> engineSetPositionCallback;
  std::function<void(std::string)> engineExecuteMoveCallback;
  std::function<void()> engineDebugCallback;
  std::function<void()> engineListMovesCallback;
  std::function<void()> engineEvaluateCallback;
  std::function<void(int)> enginePerftCallback;
  std::function<void(int)> enginePerftDivideCallback;
  std::function<unsigned long long(int)> enginePerftNodesCallback;
};
#endif
