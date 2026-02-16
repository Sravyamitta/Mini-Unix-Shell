CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = myshell
SRC = src/shell.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
