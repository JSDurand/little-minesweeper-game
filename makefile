CC=g++
CCFLAGS=-Wall -I include -L lib -l SDL2-2.0.0 -l SDL2_image -std=c++11

main: main.cpp
	$(CC) $(CCFLAGS) main.cpp -o main
