CXX = g++
CXXFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = 

SRC_DIR = src
INCLUDE_DIR = include/corex
BUILD_DIR = build

SRCS = $(SRC_DIR)/lexer/TokenType.cpp \
       $(SRC_DIR)/lexer/Lexer.cpp \
       $(SRC_DIR)/driver/Cli.cpp \
       $(SRC_DIR)/main.cpp

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/CoreX

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: all
	./$(TARGET) tokens tests/lexer_full_feature.cx

.PHONY: all clean run
