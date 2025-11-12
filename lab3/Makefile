CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = -lrt -lpthread

all: parent child1 child2

parent: parent.c
	$(CC) $(CFLAGS) -o parent parent.c $(LDFLAGS)

child1: child1.c
	$(CC) $(CFLAGS) -o child1 child1.c $(LDFLAGS)

child2: child2.c
	$(CC) $(CFLAGS) -o child2 child2.c $(LDFLAGS)

clean:
	rm -f parent child1 child2
	rm -f /dev/shm/*lab3*

.PHONY: all clean