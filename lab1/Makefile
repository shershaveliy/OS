CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGETS = parent child1 child2

all: $(TARGETS)

parent: parent.c
	$(CC) $(CFLAGS) -o parent parent.c

child1: child1.c
	$(CC) $(CFLAGS) -o child1 child1.c

child2: child2.c
	$(CC) $(CFLAGS) -o child2 child2.c

clean:
	rm -f $(TARGETS)

.PHONY: all clean