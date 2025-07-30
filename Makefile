# Compiler Flags
CC := gcc
CFLAGS := -g -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer

# Directory variables 
LIBDIR := lib
OBJ := obj
INC := include
SRC := src
TEST := tests

# Filepath Pattern Matching
LIB := $(LIBDIR)/lib.a
SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
TESTS := $(wildcard $(TEST)/*.c)
TESTBINS := $(patsubst $(TEST)/%.c, $(TEST)/bin/%, $(TESTS))

# Commands must be labeled PHONY
.PHONY: all release clean test

# Compiler Release Flags 
release: CFLAGS := -Wall -Wextra -Werror -pedantic -fsanitize=address,undefined -fno-omit-frame-pointer -O2 -DNDEBUG
release: clean $(LIB)

# Target for compilation.
all: $(LIB)

# Target / Dependencies
$(LIB): $(OBJS) | $(LIBDIR)
	$(RM) $(LIB)
	ar -cvrs $@ $^

$(OBJ)/%.o: $(SRC)/%.c $(SRC)/%.h | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST)/bin/%: $(TEST)/%.c $(LIB) | $(TEST)/bin
	$(CC) $(CFLAGS) $< $(LIB) -o $@

# Make directories if none. 
$(LIBDIR):
	mkdir $@

$(INC):
	mkdir $@

$(OBJ):
	mkdir $@

$(TEST)/bin:
	mkdir $@

# Run the tests in the bin folder and track results
test: $(LIB) $(TEST)/bin $(TESTBINS)
	@SUCCESS_COUNT=0; FAILURE_COUNT=0; \
	for test in $(TESTBINS); do \
		./$$test; \
		EXIT_CODE=$$?; \
		TEST_NAME=$(notdir $$test); \
		if [ $$EXIT_CODE -eq 0 ]; then \
			echo "\033[0;32m$$TEST_NAME: EXIT CODE: $$EXIT_CODE (SUCCESS)\033[0m"; \
			SUCCESS_COUNT=$$((SUCCESS_COUNT + 1)); \
		else \
			echo "\033[0;31m$$TEST_NAME: EXIT CODE: $$EXIT_CODE (FAILURE)\033[0m"; \
			FAILURE_COUNT=$$((FAILURE_COUNT + 1)); \
		fi; \
	done; \
	echo "\n\nTests completed"; \
	echo "SUCCESS: $$SUCCESS_COUNT"; \
	echo "FAILURE: $$FAILURE_COUNT";

clean:
	$(RM) -r $(LIBDIR) $(OBJ) $(TEST)/bin/
