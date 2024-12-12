# Compiler and flags
CXX = clang++          
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

# Automatically find all source files in the correct folder
SRC = $(shell echo Turbulence_v4/*.cpp)
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
	if exist Turbulence_v4\*.o del /f /q Turbulence_v4\*.o
	if exist $(EXE) del /f /q $(EXE)

# PGO target
pgo:
	$(CXX) $(CXXFLAGS) -fprofile-generate -o $(EXE) $(SRC)
	./$(EXE) bench
	llvm-profdata merge -output=default.profdata *.profraw
	$(CXX) $(CXXFLAGS) -fprofile-use=default.profdata -o $(EXE) $(SRC)
	del /f /q *.gcda *.profraw

# Phony targets
.PHONY: all clean pgo
