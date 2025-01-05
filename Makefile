# Compiler and flags
CXX = clang++          # Fixed to clang++
CXXFLAGS ?= -O3 -std=c++17 -Wall -Wextra # Default compiler flags

# Automatically find all source files in the correct folder
SRC = $(wildcard Turbulence_v4/*.cpp)

# Generate object file names from source files
OBJ = $(SRC:.cpp=.o)

# Output binary (default)
EXE ?= Turbulence.exe

# Default target
all: $(EXE)

# Rule to build the executable
$(EXE): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Rule to build object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	del /f /q "Turbulence_v4\*.o" "$(EXE)"

# Phony targets
.PHONY: all clean
