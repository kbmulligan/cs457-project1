CC = "g++"

all: project1

project1: project1.cpp
	g++ -o project1 project1.cpp

clean:
	rm project1.o
