CC := gcc
CFLAGS := -Wall -Wextra -std=c11 -Iinclude
LDFLAGS := `sdl2-config --cflags --libs`

SRC_DIR := src
INC_DIR := include
BIN_DIR := bin

TARGET := $(BIN_DIR)/admge

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean build
clean:
	rm -rf $(BIN_DIR)/*

# Run the emulator
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
