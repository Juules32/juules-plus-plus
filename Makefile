# Compiler settings
CXX = g++
CXXFLAGS_DEBUG = -g -Wall -Wextra -pedantic
CXXFLAGS_OPTIMIZED = -Ofast
CXXFLAGS_PUBLISH = -Ofast -static-libgcc -static-libstdc++

# Source files and output name
SRC_FILES := $(wildcard src/*.cpp)
OUTPUT = JuulesPlusPlus
OUTPUT_DIR := bin

# Compile the source files with debugging information
debug:
	@echo Compiling debugging program...
	$(CXX) $(CXXFLAGS_DEBUG) -o $(OUTPUT_DIR)/$(OUTPUT)_debug $(SRC_FILES)
	@echo Done!

# Compile with optimizations
optimized:
	@echo Compiling optimized program...
	$(CXX) $(CXXFLAGS_OPTIMIZED) -o $(OUTPUT_DIR)/$(OUTPUT)_optimized $(SRC_FILES)
	@echo Done!

# Compile with optimizations and dependencies
publish:
	@echo Compiling published program...
	$(CXX) $(CXXFLAGS_PUBLISH) -o $(OUTPUT_DIR)/$(OUTPUT) $(SRC_FILES)
	@echo Done!

# Compile and run
run: optimized
	@echo Running program...
	@bin/$(OUTPUT)_optimized
