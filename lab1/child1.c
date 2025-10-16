#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LENGTH 256

// Инвертирование строки
void invert_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // ПРИМЕЧАНИЕ: `O_WRONLY` открывает файл только для записи
	// ПРИМЕЧАНИЕ: `O_CREAT` создает запрошенный файл, если он отсутствует
	// ПРИМЕЧАНИЕ: `O_TRUNC` очищает файл перед открытием
	// ПРИМЕЧАНИЕ: при использовании `O_APPEND` последующие записи добавляются, а не перезаписываются

    int file = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (file == -1) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    
    char buffer[MAX_LENGTH];
    ssize_t bytes_read;
    
    printf("Child1: начало обработки коротких строк в файл %s\n", argv[1]);
    
    while ((bytes_read = read(STDIN_FILENO, buffer, MAX_LENGTH - 1)) > 0) {
        buffer[bytes_read] = '\0';
        
        char *line = buffer;
        char *end;
        
        while ((end = strchr(line, '\n')) != NULL) {
            *end = '\0';
            
            if (strlen(line) > 0) {
                char inverted[MAX_LENGTH];
                strcpy(inverted, line);
                invert_string(inverted);
                
                write(file, inverted, strlen(inverted));
                write(file, "\n", 1);
                
                printf("Child1: '%s' -> '%s'\n", line, inverted);
            }
            
            line = end + 1;
        }
    }
    
    close(file);
    printf("Child1: завершил работу\n");
    return 0;
}