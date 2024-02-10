# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++20 -Wall -O3 -lpthread

# Source file
SRC = a.cpp

# Executable name
TARGET = prog

# Build rule
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Run rule
run:$(TARGET)
	./$(TARGET) $(N) $(THREADS)

# Clean rule
clean:
	rm -f $(TARGET)