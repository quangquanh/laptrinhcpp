CXX ?= c++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
TARGET := snake-game

.PHONY: all run clean

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
