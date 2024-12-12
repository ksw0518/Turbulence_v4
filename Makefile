# Compiler and flags
# Compiler and flags
CXX = clang++          
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

# Automatically find all source files in the correct folder
SRC = $(shell find Turbulence_v4 -name "*.cpp")
OBJ = $(SRC:.cpp=.o)

# Output binary
EXE = Turbulence.exe

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
	-$(RM) $(OBJ)
	-$(RM) $(EXE)

# PGO target
pgo:
	$(CXX) $(CXXFLAGS) -fprofile-generate -o $(EXE) $(SRC)
	./$(EXE) bench
	llvm-profdata merge -output=default.profdata *.profraw
	$(CXX) $(CXXFLAGS) -fprofile-use=default.profdata -o $(EXE) $(SRC)
	$(RM) *.gcda *.profraw

# Phony targets
.PHONY: all clean pgo

