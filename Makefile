CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude
LDFLAGS := `sdl2-config --cflags --libs` -lm

SRC_DIR := src
INC_DIR := include
BIN_DIR := bin

TARGET := $(BIN_DIR)/admge

# one level deep \ main
SRCS := $(wildcard $(SRC_DIR)/*.c) \
$(wildcard $(SRC_DIR)/*/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

all: $(TARGET)

# build changed .o files, then use gcc
# | BIN_DIR just says that the bin directory must exist before this is run (prerequisite)
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# this is the prerequisite for the two steps above 
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# a bit obvious
clean:
	rm -rf $(BIN_DIR)/*

.PHONY: all clean run
