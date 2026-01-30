CXX = g++
CXXFLAGS = -O2 -pthread -std=c++20 -I../../renderer/include -I../../shm

SRCS = main.cpp $(wildcard ../../renderer/src/*.cpp) ../../shm/shm.cpp
OUTPUT = ./build/main

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(OUTPUT)