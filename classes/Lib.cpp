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

// converts 'e2' to 15
int Lib::getBitnumFromCoordinates(std::string coordinates)
{
	int col = coordinates.c_str()[0] - (int)'a';
	int row = coordinates.c_str()[1] - (int)'1';
	return (7 - col) + row * 8;
}

// bitshift left which supports negative values (then does a right bitshift)
unsigned long long Lib::bitShiftLeft(unsigned long long bitboard, int shift)
{
	return shift >= 0 ? bitboard << shift : bitboard >> -shift;
}

unsigned long long Lib::moveBit(unsigned long long bitboard, int from, int to)
{
	unsigned long long bitboard_from = 1ULL << from;
	return (Lib::bitShiftLeft(bitboard & bitboard_from, (to - from)) | bitboard) & (~bitboard_from);
}