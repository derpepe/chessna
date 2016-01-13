#include "Lib.h"

std::vector<std::string> &Lib::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> Lib::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

// converts 'a2' to 15
int Lib::getBitnumFromCoordinates(std::string coordinates)
{
	int col = coordinates.c_str()[0] - (int)'a';
	int row = coordinates.c_str()[1] - (int)'1';
	return (7 - col) + row * 8;
}

// converts 15 to 'a2'
std::string Lib::getCoordinatesFromBitnum(int bitnum)
{
	int file = (bitnum % 8);
	int rank = ((bitnum - file) / 8) + 1;

	std::string result;
	result += (char) ((int)'h' - file);
	result += std::to_string(rank);
	
	return result;
}


// bitshift left which supports negative values (then does a right bitshift)
unsigned long long Lib::bitShiftLeft(unsigned long long bitboard, int shift)
{
	return shift >= 0 ? bitboard << shift : bitboard >> -shift;
}

unsigned long long Lib::moveBit(unsigned long long bitboard, int from, int to)
{
	int delta = to - from;
	unsigned long long bitboard_from = 1ULL << from;
	unsigned long long bitboard_to = 1ULL << to;
	
	unsigned long long clearedToField = bitboard & (~bitboard_to); // removes bit at 'to'
	unsigned long long movedBit = Lib::bitShiftLeft(clearedToField & bitboard_from, delta) | clearedToField;  // moves bit from 'from' to 'to'
	return movedBit & (~bitboard_from); // removes bit at 'from'
}