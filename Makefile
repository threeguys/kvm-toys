

CC=gcc
DEPS=$(wildcard *.h)
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

vm: $(OBJS)
	$(CC) -o vm $(OBJS)

all: vm

clean:
	rm -f vm *.o
