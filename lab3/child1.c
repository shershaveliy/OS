#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>

#define SHM_SIZE 4096
#define BUFFER_SIZE 1024

struct shared_data{
    uint32_t length1;
    uint32_t length2;
    bool parent_finished;
    char text1[BUFFER_SIZE];
    char text2[BUFFER_SIZE];
};

void invert_string(char *str){
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main(int argc, char *argv[]){
    if (argc != 4){
        fprintf(stderr, "Usage: %s shm_name sem_name filename\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    char *shm_name = argv[1];
    char *sem_name = argv[2];
    char *filename = argv[3];
    
    printf("Child1: запущен для файла %s\n", filename);
    
    int shm_fd = shm_open(shm_name, O_RDWR, 0);
    if (shm_fd == -1){
        perror("Child1: shm_open failed");
        exit(EXIT_FAILURE);
    }
    
    struct shared_data *shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED){
        perror("Child1: mmap failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    sem_t *sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED){
        perror("Child1: sem_open failed");
        munmap(shm, SHM_SIZE);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    FILE *file = fopen(filename, "w");
    if (file == NULL){
        perror("Child1: fopen failed");
        sem_close(sem);
        munmap(shm, SHM_SIZE);
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    bool running = true;
    printf("Child1: начало обработки коротких строк в файл %s\n", filename);
    
    while (running){
        sem_wait(sem);
        
        if (shm->length1 > 0){
            char inverted[BUFFER_SIZE];
            strcpy(inverted, shm->text1);
            invert_string(inverted);
            
            fprintf(file, "%s\n", inverted);
            // fflush - сбрасывает буфер файла (данные сразу записываются)
            fflush(file);
            
            printf("Child1: '%s' -> '%s'\n", shm->text1, inverted);
            
            // Сброс длины для сигнализации о обработке
            shm->length1 = 0;
        }
        
        if (shm->parent_finished && shm->length1 == 0){
            running = false;
        }
        
        sem_post(sem);
        
        usleep(50000); // Небольшая задержка для уменьшения нагрузки на CPU
    }
    
    fclose(file);
    sem_close(sem);
    munmap(shm, SHM_SIZE);
    close(shm_fd);
    
    printf("Child1: завершил работу\n");
    return 0;
}