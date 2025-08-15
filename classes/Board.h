#ifndef BOARD_H
#define BOARD_H

#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include "Lib.h"
#include <vector>
#include <string>

class MoveGenerator;

class Board
{
public:
	Board();
	Board(const Board& other);
	void startpos();

	void loadFen(const std::string&);
	void loadFen(const std::string&, char, const std::string&, const std::string&, long, long);
	std::string getDump();
	
	void checkConsistency();
	
	void executeMove(std::string move, bool incrementCounters = true);
	
	std::vector<std::string> getAllMoves();

	char getPlayerToMove() const { return playerToMove; }
			
// private: //
	unsigned long long blacks;
	unsigned long long whites;
	
	unsigned long long kings;
	unsigned long long queens;
	unsigned long long rooks;
	unsigned long long bishops;
	unsigned long long knights;
	unsigned long long pawns;
	
	char playerToMove;
	
	bool casteling_K;
	bool casteling_k;
	bool casteling_Q;
	bool casteling_q;

	std::string enPassant;

	long halfmoves;
	long currentMove;
};

#endif
