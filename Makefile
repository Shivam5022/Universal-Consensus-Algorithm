CXX = g++
CXXFLAGS = -std=c++20 -Wall -O3 -lpthread
SRC = LF-universalConsensus.cpp
# SRC = WF-universalConsensus.cpp
TARGET = prog

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run:$(TARGET)
	./$(TARGET) $(N)

clean:
	rm -f $(TARGET)