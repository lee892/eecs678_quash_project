Quash: main.o Quash.o
	g++ -std=c++11 -g -Wall main.o Quash.o -o Quash
main.o: main.cpp Quash.h
	g++ -std=c++11 -g -Wall -c main.cpp
Quash.o: Quash.h Quash.cpp
	g++ -std=c++11 -g -Wall -c Quash.cpp

clean:
	rm *.o Quash