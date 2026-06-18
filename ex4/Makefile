CC=gcc
DB=gdb

ARCH=64

CFLAGS=-m$(ARCH) -Wall -Wextra -Wpedantic -Wconversion -funsigned-char -I$(INCLUDE_DIR) -I$(HASH_TABLE_INCLUDE)
DEBUG_FLAGS=-g$(DB) -DDEBUG -fsanitize=address
RELEASE_FLAGS=-O2 -DNDEBUG

SRC_DIR=src
OBJ_DIR=obj
BIN_DIR=bin
INCLUDE_DIR=include
TEST_DIR=test

HASH_TABLE_INCLUDE=../ex2/include
HASH_TABLE_SRC=../ex2/src/hash_table.c

SRCS=$(SRC_DIR)/graph.c $(SRC_DIR)/bfs.c $(HASH_TABLE_SRC)
OBJS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

MAIN_SRCS=$(SRC_DIR)/main.c
MAIN_OBJS=$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(MAIN_SRCS))

TEST_SRCS=$(wildcard $(TEST_DIR)/*.c)
TEST_OBJS=$(patsubst $(TEST_DIR)/%.c, $(OBJ_DIR)/%.o, $(TEST_SRCS))

BIN=$(BIN_DIR)/main_ex4
TEST_BIN=$(BIN_DIR)/test_ex4

all: release test

debug: CFLAGS+=$(DEBUG_FLAGS)
debug: $(BIN)

test: CFLAGS+=$(DEBUG_FLAGS)
test: $(TEST_BIN)

release: CFLAGS+=$(RELEASE_FLAGS)
release: clean
release: $(BIN)

$(OBJ_DIR):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJS) $(MAIN_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_BIN): $(OBJS) $(TEST_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*
