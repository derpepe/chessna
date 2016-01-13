#ifndef LIB_H
#define LIB_H

#include <string>
#include <sstream>
#include <vector>

class Lib
{
public:
	static std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
	static std::vector<std::string> split(const std::string &s, char delim);
	static int getBitnumFromCoordinates(std::string coordinates);
	static std::string getCoordinatesFromBitnum(int bitnum);
	static unsigned long long bitShiftLeft(unsigned long long bitboard, int shift);
	static unsigned long long moveBit(unsigned long long bitboard, int from, int to);
	static int getFile(int bitnum);
	static int getRank(int bitnum);
};

#endif