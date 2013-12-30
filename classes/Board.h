#ifndef BOARD_H
#define BOARD_H

#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <iostream>

class Board
{
public:
	Board();
	void clear();
	
	void loadFen(std::string);
	std::string getDump();
	
private:
	unsigned long long blacks;
	unsigned long long whites;
	
	unsigned long long kings;
	unsigned long long queens;
	unsigned long long rooks;
	unsigned long long bishops;
	unsigned long long knights;
	unsigned long long pawns;
	
	char playerToMove;
};

#endif
