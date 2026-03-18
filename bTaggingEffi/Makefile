# Compiler and ROOT configuration
CXX = g++
CXXFLAGS = -O2 -Wall -fPIC `root-config --cflags`
LDFLAGS = `root-config --libs`

# Source and Target
SRC = makeBtagEff_All.cpp
TARGET = makeBtagEff_All

# Build rule
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)
