CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
BUILD_DIR = build
TARGET = $(BUILD_DIR)/CoreX

SOURCES = $(wildcard src/lexer/*.cpp) $(wildcard src/driver/*.cpp)
OBJECTS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

$(BUILD_DIR)/%.o: src/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET) tokens tests/lexer_full_feature.cx

.PHONY: all clean run
