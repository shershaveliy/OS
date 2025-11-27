CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -ldl

all: library1.so library2.so prog1 prog2

library1.so: library1.c library.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ library1.c

library2.so: library2.c library.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ library2.c

# Программа со статической линковкой
prog1: program1.c library1.so
	$(CC) $(CFLAGS) -o $@ program1.c -L. -Wl,-rpath,. library1.so

# Программа с динамической загрузкой
prog2: program2.c
	$(CC) $(CFLAGS) -o $@ program2.c $(LDFLAGS)

clean:
	rm -f *.so prog1 prog2

.PHONY: all clean