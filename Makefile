# Compiler settings
CXX = g++
CXXFLAGS_DEBUG = -g
CXXFLAGS_OPTIMIZED = -Ofast

# Source files and output name
SRC_FILES := $(wildcard *.cpp)
OUTPUT = ChessPlusPlus

# Default target
all: $(OUTPUT)

# Compile the source files with debugging information
debug: $(SRC_FILES)
	@$(CXX) $(CXXFLAGS_DEBUG) -o $(OUTPUT)_debug $(SRC_FILES)

# Compile the source files with optimization
optimized: $(SRC_FILES)
	@$(CXX) $(CXXFLAGS_OPTIMIZED) -o $(OUTPUT) $(SRC_FILES)

run: debug
	@./$(OUTPUT)_debug

# Clean the generated executables
clean:
	del $(OUTPUT)_debug.exe $(OUTPUT).exe