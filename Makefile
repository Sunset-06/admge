CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -std=c11
CXXFLAGS := -Wall -Wextra -std=c++17
SDL_CFLAGS := `sdl2-config --cflags`
SDL_LIBS := `sdl2-config --libs`

LDFLAGS := $(SDL_LIBS) -lm

INCLUDES := -Iinclude \
            -Ilibraries/imgui/include \
            -Ilibraries/tinyfiledialogs/include

SRC_DIR := src
INC_DIR := include
LIB_DIR := libraries
BIN_DIR := bin

TARGET := $(BIN_DIR)/admge

# three types of srcs
MAIN_SRCS := $(wildcard $(SRC_DIR)/*.c) \
$(wildcard $(SRC_DIR)/*/*.c)

TINYFD_SRCS := $(wildcard $(LIB_DIR)/tinyfiledialogs/src/*.c)

IMGUI_SRCS := $(wildcard $(LIB_DIR)/imgui/src/*.cpp)

SRCS := $(MAIN_SRCS) $(TINYFD_SRCS) $(IMGUI_SRCS)

# PATSUBST - (pattern, replacement, text)
# 		   - find %.c, replace with bin/%.o, in all srcs  
OBJS := $(patsubst %.c,$(BIN_DIR)/%.o,$(MAIN_SRCS)) \
        $(patsubst %.c,$(BIN_DIR)/%.o,$(TINYFD_SRCS)) \
        $(patsubst %.cpp,$(BIN_DIR)/%.o,$(IMGUI_SRCS))


all: $(TARGET)

# Linking
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	
# Compile C
$(BIN_DIR)/%.o: %.c | $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(INCLUDES) -c $< -o $@

# Compile C++
$(BIN_DIR)/%.o: %.cpp | $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) $(INCLUDES) -c $< -o $@

# this is the prerequisite for the two steps above 
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# a bit obvious
clean:
	rm -rf $(BIN_DIR)/*

# tests
test-%: all
	./$(TARGET) ./roms/$*-test.gb

.PHONY: all clean test
