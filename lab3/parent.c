#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>

#define SHM_SIZE 4096 //Размер для виртуальной памяти
#define BUFFER_SIZE 1024 //Размер строк для ввода

struct shared_data{
    uint32_t length1;// длина строки для child1
    uint32_t length2; // длина строки для child2
    bool parent_finished; // флаг завершения родителя
    char text1[BUFFER_SIZE];// строка для child1
    char text2[BUFFER_SIZE];// строка для child2
};

//Уникальное имя для системных ресурсов
void generate_resource_names(char *shm_name, char *sem_name, size_t size){
    pid_t pid = getpid();
    snprintf(shm_name, size, "/lab3-shm-%d", pid);
    snprintf(sem_name, size, "/lab3-sem-%d", pid);
}

int main(){
    char shm_name[64], sem_name[64];
    generate_resource_names(shm_name, sem_name, sizeof(shm_name));
    
    printf("SHM name: %s\n", shm_name);
    printf("SEM name: %s\n", sem_name);
    

    char filename1[256], filename2[256];
    printf("Введите имя файла для child1: ");
    if(fgets(filename1, sizeof(filename1), stdin) == NULL){
        perror("fgets failed");
        exit(EXIT_FAILURE);
    }

    filename1[strcspn(filename1, "\n")] = 0;
    
    printf("Введите имя файла для child2: ");
    if(fgets(filename2, sizeof(filename2), stdin) == NULL){
        perror("fgets failed");
        exit(EXIT_FAILURE);
    }

    filename2[strcspn(filename2, "\n")] = 0;

    
    // Создание shared memory (shm_open файловый дескриптор или -1 при ошибке)
    int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if(shm_fd == -1){
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }
    //Устанавливаем размер shared memory
    if(ftruncate(shm_fd, SHM_SIZE) == -1){
        perror("ftruncate failed");
        shm_unlink(shm_name);//Удаление объекта
        exit(EXIT_FAILURE);
    }
    
    //mmap - отображает shared memory в адресное пространство процесса
    struct shared_data *shm = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    //MAP_SHARED - изменения видны другим процессам, 0 - смещение от начала
    if (shm == MAP_FAILED){
        perror("mmap failed");
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }
    
    // Инициализация shared memory
    shm->length1 = 0;
    shm->length2 = 0;
    shm->parent_finished = false;
    
    // Семафор
    // 1 - начальное значение семафора (доступен)
    // Возвращает: указатель на семафор или SEM_FAILED
    sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL, 0600, 1);
    if(sem == SEM_FAILED){
        perror("sem_open failed");
        munmap(shm, SHM_SIZE); //убирает отображение памяти
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }
    

    pid_t child1 = fork();
    if (child1 == -1){
        perror("fork failed");
        sem_close(sem);
        sem_unlink(sem_name);
        munmap(shm, SHM_SIZE);
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }
    
    if (child1 == 0){
        //Массив аргументов командной строки для execv
        char *args[] = {"./child1", shm_name, sem_name, filename1, NULL};
        execv(args[0], args);
        perror("execv failed");
        _exit(EXIT_FAILURE);
    }
    
    pid_t child2 = fork();
    if (child2 == -1){
        perror("fork failed");
        sem_close(sem);
        sem_unlink(sem_name);
        munmap(shm, SHM_SIZE);
        shm_unlink(shm_name);
        exit(EXIT_FAILURE);
    }
    
    if (child2 == 0){
        char *args[] = {"./child2", shm_name, sem_name, filename2, NULL};
        execv(args[0], args);
        perror("execv failed");
        _exit(EXIT_FAILURE);
    }
    
    printf("Вводите строки (Ctrl+D для завершения):\n");
    char buffer[BUFFER_SIZE];
    
    while (fgets(buffer, sizeof(buffer), stdin) != NULL){
        buffer[strcspn(buffer, "\n")] = 0;
        size_t len = strlen(buffer);
        
        sem_wait(sem);// Захват семафора
        
        if (len > 10){
            shm->length2 = len;
            memcpy(shm->text2, buffer, len + 1);
            printf("Parent: отправлена длинная строка в child2: '%s'\n", buffer);
        }
        else{
            shm->length1 = len;
            memcpy(shm->text1, buffer, len + 1);
            printf("Parent: отправлена короткая строка в child1: '%s'\n", buffer);
        }
        
        sem_post(sem);// Освобождение семафора
        
        usleep(10000);//Это время для того чтобы дети смогли обработать данные
    }
    
    sem_wait(sem);
    shm->parent_finished = true;
    sem_post(sem);
    
    printf("Parent: ожидание завершения дочерних процессов...\n");
    
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    
    sem_close(sem);
    sem_unlink(sem_name);
    munmap(shm, SHM_SIZE);
    shm_unlink(shm_name);
    close(shm_fd);
    
    printf("Родительский процесс завершен.\n");
    return 0;
}