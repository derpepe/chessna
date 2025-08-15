#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "Board.h"
#include <string>

class Evaluator
{
public:
	static int evaluate(Board&, const std::string&);
};

#endif
