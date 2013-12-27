#include <iostream>
class Board
{
public:
	long long blacks;
	long long whites;
	
	long long kings;
	long long queens;
	long long rooks;
	long long bishops;
	long long knights;
	long long pawns;
};


int main(int argc, char** args)
{
	Board *board = new Board();
	std::cout << sizeof(board->kings) << " (should be 8 byte)";
	return 0;
}
