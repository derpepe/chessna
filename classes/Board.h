#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <sstream>
#include <iterator>
#include <vector>


class Board
{
public:
	Board();
	void clear();
	
	void loadFen(std::string);
	
private:
	long long blacks;
	long long whites;
	
	long long kings;
	long long queens;
	long long rooks;
	long long bishops;
	long long knights;
	long long pawns;
};

#endif