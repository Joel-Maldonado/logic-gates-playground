CC = clang
SDKROOT = $(shell xcrun --show-sdk-path)
SYSROOT_FLAGS = -isysroot $(SDKROOT)
CFLAGS = -Wall -Wextra -std=c99 -Isrc -isystem include -Weverything -Werror -Wno-covered-switch-default -Wno-unsafe-buffer-usage $(SYSROOT_FLAGS)
LDFLAGS = $(SYSROOT_FLAGS) -Llib -lraylib -framework IOKit -framework Cocoa -framework OpenGL

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRC = $(wildcard $(SRC_DIR)/*.c)
APP_SRC = $(filter-out $(SRC_DIR)/main.c,$(SRC))
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = $(BIN_DIR)/mvld

TEST_DIR = tests
TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_BIN = $(BIN_DIR)/test_main

all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(APP_SRC) $(TEST_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean test
