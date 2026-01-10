CXX = g++
CXXFLAGS = -O2 -pthread -std=c++20 -I../shm

SRCS = ./main.cpp ../shm/shm.cpp
OUTPUT = ./build/main

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(OUTPUT) -l wiringPi -l zmq

# g++ ./main.cpp ../shm/shm.cpp -I ../shm -o build/test -l wiringPi -std=c++20
# g++ -l wiringPi -std=c++20 -I ../shm ./main.cpp ../shm/shm.cpp -o build/test