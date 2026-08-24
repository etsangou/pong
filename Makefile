CXX = g++
CXXFLAGS = -std=c++17 -O2
SRC = src/main.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = pong

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) `sdl2-config --cflags --libs`

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJ)
