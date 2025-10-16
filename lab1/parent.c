#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LENGTH 256

int main() {
    int pipe1[2];  // Для коротких строк (<10 символов)
    int pipe2[2];  // Для длинных строк (>=10 символов)
    
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }
    
    char filename1[MAX_LENGTH], filename2[MAX_LENGTH];
    
    printf("Введите имя файла для child1: ");
    if (fgets(filename1, MAX_LENGTH, stdin) == NULL) {
        perror("fgets failed");
        exit(EXIT_FAILURE);
    }
    filename1[strcspn(filename1, "\n")] = 0;
    
    printf("Введите имя файла для child2: ");
    if (fgets(filename2, MAX_LENGTH, stdin) == NULL) {
        perror("fgets failed");
        exit(EXIT_FAILURE);
    }
    filename2[strcspn(filename2, "\n")] = 0;
    
    pid_t child1 = fork();
    
    if (child1 == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (child1 == 0) {
        // child1 - инвертирует строки из pipe1
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        
        // Перенаправляем stdin на pipe1[0]
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[0]);
        
        execl("./child1", "child1", filename1, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    }
    
    pid_t child2 = fork();
    
    if (child2 == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    
    if (child2 == 0) {
        close(pipe2[1]);
        close(pipe1[0]);
        close(pipe1[1]);
        
        dup2(pipe2[0], STDIN_FILENO);
        close(pipe2[0]);
        
        execl("./child2", "child2", filename2, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    }
    
    close(pipe1[0]);
    close(pipe2[0]);
    
    char buffer[MAX_LENGTH];
    
    printf("Вводите строки (Ctrl+D для завершения):\n");
    
    while (fgets(buffer, MAX_LENGTH, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        
        if (strlen(buffer) > 10) {
            write(pipe2[1], buffer, strlen(buffer));
            write(pipe2[1], "\n", 1);
        } else {
            write(pipe1[1], buffer, strlen(buffer));
            write(pipe1[1], "\n", 1);
        }
    }
    
    close(pipe1[1]);
    close(pipe2[1]);
    
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    
    printf("Родительский процесс завершен.\n");
    return 0;
}