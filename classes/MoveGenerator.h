#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <vector>
#include <string>

class Board;

class MoveGenerator
{
public:
	std::vector<std::string> getAllMoves(Board&);
	bool isSquareAttacked(Board&, int square, char byPlayer);

private:
	void addMoveToList(Board&, std::vector<std::string> *moves, int from, int to);
	void addMoveToList(Board&, std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty);
	void addMoveToListDiag(Board&, std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty);
	void addMoveToList(Board&, std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, int maxdist);
	void addMoveToList(Board&, std::vector<std::string> *moves, int from, int to, std::vector<int> checkForEmpty, std::string promotion);
	void addSlidingMoves(Board&, std::vector<std::string> *moves, int from, const std::vector<int>& directions);
};

#endif
