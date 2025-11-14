# Compiler
CXX = g++
CXXFLAGS = -I/opt/homebrew/include -std=c++17

LDFLAGS = -L/opt/homebrew/lib -lsfml-graphics -lsfml-window -lsfml-system

TARGET = physicSims

SRC = $(wildcard src/*.cpp) $(wildcard src/sims/*.cpp)

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(CXXFLAGS) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
