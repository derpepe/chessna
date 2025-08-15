#ifndef EVALUATION_H
#define EVALUATION_H

#include "Board.h"
#include <string>

class Evaluation
{
public:
        static int evaluate(Board&, const std::string&);

private:
        static int evaluateColorless(Board&, const std::string&);
};

#endif
