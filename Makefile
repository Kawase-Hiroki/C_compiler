CC = gcc
CFLAGS = -g
TARGET = main
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

ASM = foo.s
EXE = foo

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)



test: $(TARGET)
	chmod a+x test.sh
	./test.sh

clean:
	rm -f $(TARGET) $(ASM) $(EXE)  *.o *~ tmp*

.PHONY: all test clean