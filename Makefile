CXX = g++
CXXFLAGS = -std=c++20 -Wall -O3 -lpthread
SRC = LF-universalConsensus.cpp
# SRC = a.cpp

TARGET = prog

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run:$(TARGET)
	./$(TARGET) $(N) $(THREADS)

clean:
	rm -f $(TARGET)