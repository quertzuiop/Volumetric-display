CXX = g++
CXXFLAGS = -O2 -pthread -std=c++20 -I../../renderer/include
# List all sources: your main file + the renderer library files
SRCS = main.cpp $(wildcard ../../renderer/src/*.cpp)
OUTPUT = ./build/main

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(OUTPUT)