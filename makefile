CC = "g++"

all: project1

project1: project1.cpp
	g++ -Wall -std=c++0x -o chat project1.cpp

clean:
	rm project1.o
