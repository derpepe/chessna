GPP=g++

GPPOPTS=-Wall -Werror -std=c++11 -O3 -pthread

.PHONY: all clean

all: bin/chessna

bin/chessna: bin/Board.o bin/Comm.o bin/Engine.o bin/Lib.o bin/Uci.o bin/main.o bin/Perft.o bin/MoveGenerator.o bin/Evaluation.o
	$(GPP) $(GPPOPTS) -o bin/chessna bin/*.o

bin/%.o: classes/%.cpp classes/*.h
	$(GPP) $(GPPOPTS) -o $@ -c $<

clean:
	rm -f -- bin/chessna bin/*.o
