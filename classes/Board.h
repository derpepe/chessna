#ifndef BOARD_H
#define BOARD_H

#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>

class Board
{
public:
	Board();
	void clear();
	
	void loadFen(std::string);
	std::string getDump();
	
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
