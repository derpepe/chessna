#pragma once
#include <string>

class Tests {
public:
    bool runAll();
private:
    bool testFenLoading();
    bool testPerft();
    bool testEvaluation();
    std::string extractFen(const std::string& dump);
};
