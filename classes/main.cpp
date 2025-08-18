#include "Uci.h"
#include "Engine.h"
#include "Comm.h"

int main() {
    Comm comm;
    Engine engine(&comm);
    Uci uci(&comm);   // Uci erwartet Comm*

    // Callbacks sauber registrieren (Uci triggert diese)
    comm.registerEngineGoCallback([&](GoParams p){ engine.go(p); });
    comm.registerEngineStopCallback([&](){ engine.stop(); });

    // UCI "position": setPosition(FEN/startpos) + executeMove (einzeln)
    comm.registerEngineSetPositionCallback([&](std::string fen){
        engine.setPosition(fen, {});
    });
    comm.registerEngineExecuteMoveCallback([&](std::string move){
        engine.applyMove(move);
    });

    // Perft-Hooks
    comm.registerEnginePerftCallback([&](int d){ engine.perft(d); });
    comm.registerEnginePerftDivideCallback([&](int d){ engine.perftDivide(d); });
    comm.registerEnginePerftNodesCallback([&](int d)->unsigned long long { return engine.perftNodes(d); });

    // Uci übernimmt I/O-Loop intern
    uci.run();
    return 0;
}
