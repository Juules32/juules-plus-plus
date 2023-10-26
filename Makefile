# Compiler settings
CXX = g++
CXXFLAGS_DEBUG = -g -Wall -Wextra -pedantic
CXXFLAGS_OPTIMIZED = -Ofast
CXXFLAGS_PUBLISH = -Ofast -static-libgcc -static-libstdc++

# Source files and output name
SRC_FILES := $(wildcard *.cpp)
OUTPUT = JuulesPlusPlus
OUTPUT_DIR := C:\\Users\\benja\\Documents\\Code\\lichess-bot\\engines

# Default target
all: $(OUTPUT)

# Compile the source files with debugging information
debug: $(SRC_FILES)
	@echo Compiling debugging program...
	$(CXX) $(CXXFLAGS_DEBUG) -o $(OUTPUT)_debug $(SRC_FILES)
	@echo Done!

# Compile the source files with optimization
optimized: $(SRC_FILES)
	@echo Compiling optimized program...
	$(CXX) $(CXXFLAGS_OPTIMIZED) -o $(OUTPUT)_optimized $(SRC_FILES)
	@echo Done!

publish: $(SRC_FILES)
	@echo Compiling published program...
	$(CXX) $(CXXFLAGS_PUBLISH) -o $(OUTPUT) $(SRC_FILES)
	@echo Done!

dir: $(SRC_FILES)
	@echo Compiling optimized program for lichess engine folder...
	$(CXX) $(CXXFLAGS_PUBLISH) $(SRC_FILES) -o $(OUTPUT_DIR)/$(OUTPUT)
	@echo Done!

run: optimized
	@echo Running program...
	@./$(OUTPUT)_optimized
