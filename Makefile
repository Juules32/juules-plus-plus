# Detect the operating system
UNAME_S := 

# Set the compiler and flags based on OS
ifeq ($(shell uname -s),Linux)
    EMCC = ./emcc
else
    EMCC = emcc
endif

# Compiler settings
CXX = g++
CXXFLAGS_DEBUG = -g -Wall -Wextra -pedantic
CXXFLAGS_OPTIMIZED = -Ofast
CXXFLAGS_PUBLISH = -Ofast -static-libgcc -static-libstdc++

# Source files and output name
SRC_FILES := src/main.cpp
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

webassembly:
	@echo Compiling to WebAssembly...
	$(EMCC) -O3 src/web_build.cpp -o web/JuulesPlusPlus.js 										   \
	-s EXPORTED_FUNCTIONS=_setup,_make_move,_engine_move,_valid_move,_valid_targets,_make_move_str \
	-s EXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString 										   \
	-s MODULARIZE=1 																			   \
	-s WASM=1 																					   \
	-s WASM_BIGINT=1																			   \
	-s TOTAL_STACK=512mb
