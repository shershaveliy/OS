CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -O2
TARGET = main
SOURCES = main.c parallel.c
HEADERS = parallel.h

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) 10000 4 --print

.PHONY: clean run