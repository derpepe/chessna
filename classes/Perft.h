#ifndef PERFT_H
#define PERFT_H

#include <iostream>

class Board;

struct PerftResult {
	unsigned long long nodes = 0;
	unsigned long long captures = 0;
	unsigned long long en_passant = 0;
	unsigned long long castles = 0;
	unsigned long long promotions = 0;
	unsigned long long checks = 0;
	unsigned long long checkmates = 0;

	PerftResult& operator+=(const PerftResult& rhs)
	{
		this->nodes += rhs.nodes;
		this->captures += rhs.captures;
		this->en_passant += rhs.en_passant;
		this->castles += rhs.castles;
		this->promotions += rhs.promotions;
		this->checks += rhs.checks;
		this->checkmates += rhs.checkmates;
		return *this;
	}
};

class Perft
{
public:
	PerftResult perft(Board&, int);
	void perftDivide(Board&, int);
};

#endif
