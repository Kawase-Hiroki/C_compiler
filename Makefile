CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L
TARGET = main
SRCS = $(wildcard src/*.c)
ASM = foo.s
EXE = foo
TEST_RUNNER = test_runner

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

test: $(TARGET) compiler_tests.c
	$(CC) $(CFLAGS) -o $(TEST_RUNNER) compiler_tests.c
	./$(TEST_RUNNER)

clean:
	rm -f $(TARGET) $(ASM) $(EXE) $(TEST_RUNNER) test_input.c *.o *~ tmp*

.PHONY: all test clean
