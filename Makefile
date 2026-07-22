# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -pthread

# Source files
SRC = src/main.cpp \
      src/server.cpp \
      src/command.cpp \
      src/database.cpp \
      src/persistence.cpp \
      src/resp.cpp

# Output executable
TARGET = server

# Build project
all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# Run server
run: all
	./$(TARGET)

# Clean generated files
clean:
	rm -f $(TARGET)