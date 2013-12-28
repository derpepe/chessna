#include <iostream>
#include <thread>
#include <functional>
#include "Board.h"
#include "Communication.h"


int main(int argc, char** args)
{
	Communication *comm = new Communication();

	// initialize asynchronous communication
	std::thread iothread(std::bind(&Communication::mainLoop, comm));

	Board *board = new Board();
	while(true) {
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		for (int i = 0; i < 1000000; i++) {}
		comm->engineSays("hello");
	}
	
	iothread.join();
	return 0;
}
