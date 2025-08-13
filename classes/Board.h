#ifndef BOARD_H
#define BOARD_H

#include <sstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Lib.h"

class Board
{
public:
	Board();
	Board(const Board&);
	void startpos();
	
	void loadFen(std::string);
	void loadFen(std::string, char, std::string, std::string, long, long);
	std::string getDump();
	
	void checkConsistency();
	
	void executeMove(std::string move, bool incrementCounters = true);
	
	std::vector<std::string> getAllMoves();
	void addMoveToList(std::vector<std::string> *moves, int from, int to);
	void addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty);
	void addMoveToListDiag(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty);
	void addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, int maxdist);
	void addMoveToList(std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, std::string promotion);

	unsigned long long perft(int);
			
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
	
	bool casteling_K;
	bool casteling_k;
	bool casteling_Q;
	bool casteling_q;

	std::string enPassant;

	long halfmoves;
	long currentMove;
};

#endif
