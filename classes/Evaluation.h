#ifndef EVALUATION_H
#define EVALUATION_H

#include "Board.h"

class Evaluation
{
public:
        static int evaluate(Board&);

private:
        static int evaluateColorless(Board&);
};

#endif
