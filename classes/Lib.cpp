#include "Lib.h"

std::vector<std::string> &Lib::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim))
    {
        if (!item.empty())
        {
            elems.push_back(item);
        }
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
	int col = coordinates.c_str()[0] - 'a';
	int row = coordinates.c_str()[1] - '1';
	return row * 8 + col;
}

// converts 15 to 'a2'
std::string Lib::getCoordinatesFromBitnum(int bitnum)
{
	int rank = bitnum / 8;
	int file = bitnum % 8;
	std::string result = "";
	result += (char)('a' + file);
	result += (char)('1' + rank);
	return result;
}

int Lib::getRank(int bitnum)
{
	return bitnum / 8;
}

int Lib::getFile(int bitnum)
{
	return bitnum % 8;
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

std::string Lib::formatThousands(unsigned long long value)
{
        std::string s = std::to_string(value);
        int insertPosition = s.length() - 3;
        while (insertPosition > 0)
        {
                s.insert(insertPosition, ".");
                insertPosition -= 3;
        }
        return s;
}
