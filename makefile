# Makefile for OS Emulator

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++20 -Wall -pthread
LDFLAGS =

# Source files
SRCS = main.cpp Emulator.cpp Scheduler.cpp Process.cpp InstructionGenerator.cpp Marquee.cpp ScreenInfo.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = emulator

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
