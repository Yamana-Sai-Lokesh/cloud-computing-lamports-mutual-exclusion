# Compiler
CC := g++

# Compiler flags
CFLAGS := -Wall -Wextra -pedantic -std=c++11

# Source files directory
SRC_DIR := lib

# Source files1
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS := $(SRCS:.cpp=.o)

# Target executable
TARGET := main

# Rule to build the target executable
$(TARGET): $(OBJS) main.cpp
	$(CC) $(CFLAGS) $^ -o $@

# Rule to build object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)