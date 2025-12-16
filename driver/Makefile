CXX = g++
CXXFLAGS = -O2 -pthread -std=c++20 -I../shm -I ./include

SRCS = ./src/driver.cpp ./src/displayControl.cpp ../shm/shm.cpp
OUTPUT = ./build/main

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(OUTPUT)