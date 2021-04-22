CC=gcc
CFLAGS=-Wall -Wno-deprecated-declarations -I. -pthread -std=c99
DEPS = game.h
OBJ = game.o main.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
		rm *.o

all: main clean
