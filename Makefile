MAIN_NAME = main
OUT_NAME = main_out

INCLUDES = include
LINKS = -lpthread
CFLAGS = -Wall -I$(INCLUDES)

CC = gcc
BIN = bin
SRC = src
SRCS = $(wildcard $(SRC)/**/*.c $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(BIN)/%.o, $(SRCS))
DEPS = $(wildcard $(INCLUDES)/*.h)

TEST_FOLDER = test
TEST_OBJS = $(filter-out $(BIN)/$(MAIN_NAME).o, $(OBJS))
TEST_SRC = $(wildcard $(TEST_FOLDER)/*.c)
TEST_EXECUTABLES = $(TEST_SRC:.c=)
UNITY_SRC = $(TEST_FOLDER)/Unity/src/unity.c
TEST_FLAGS = -I$(INCLUDES) $(DEBUG_FLAGS)

.PHONY: test debug clean clean-objs

all: clean $(OBJS) $(BIN)/$(OUT_NAME)
all: clean-objs

debug: CFLAGS += -g -DDEBUG
debug: all

$(BIN)/$(OUT_NAME): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS) $(LINKS)

$(BIN)/%.o: $(SRC)/%.c $(DEPS)
	@mkdir -p $(@D)
	$(CC) -c -o $@ $< $(CFLAGS) $(LINKS)

clean:
	@rm -rf $(BIN)
	@find . -name "*.o" -type f -delete
	@find . -name "*.log" -type f -delete
	@find . -name "*.bin" -type f -delete

clean-objs:
	@echo "[i] Cleaning up object files..."
	@find $(BIN) -name "*.o" -type f -delete
	@find $(BIN) -type d -empty -delete
	@echo "[i] Cleanup complete"

test: $(TEST_OBJS)
	@echo "[i] Compiling tests..."
	@$(foreach test, $(TEST_SRC), \
	$(CC) $(TEST_FLAGS) $(test) -o $(test:.c=.bin) $(TEST_OBJS) $(UNITY_SRC) $(LINKS); \
	)
	@echo "[i] Compilation tests complete"