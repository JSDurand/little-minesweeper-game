CC=g++
CCFLAGS=-Wall -I include -L lib -l SDL2-2.0.0 -l SDL2_image -l SDL2_ttf -std=c++11

main: main.cpp
	$(CC) $(CCFLAGS) main.cpp -o main
