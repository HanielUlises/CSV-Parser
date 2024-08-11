# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -Wextra -I.

# Source files
SRC = example/main.cpp

# Header files
HEADERS = CSVParser.h example/class_ex.h

# Output executable
TARGET = example_program

# Build target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Clean the build
clean:
	rm -f $(TARGET)
