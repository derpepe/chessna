#include <iostream>
using namespace std;

class board
{
public:
	long blacks;
	long whites;
	
	long kings;
	long queens;
	long rooks;
	long bishops;
	long knights;
	long pawns;
};


int main()
{
	board b;
	cout << b.kings;
	return 0;
}
