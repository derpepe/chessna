#ifndef BOARD_H
#define BOARD_H

class Board
{
public:
	Board();
	void clear();
	
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