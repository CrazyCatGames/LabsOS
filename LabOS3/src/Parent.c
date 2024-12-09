#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

#define SHM_NAME "/shared_memory"
#define SEM_NAME "/sync_semaphore"
#define BUFFER_SIZE 1024
#define NUM_LINES 100

int main() {
    int shm_fd;
    char *shared_mem;
    sem_t *semaphore;

    // Создание разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shm_fd, BUFFER_SIZE * NUM_LINES) == -1) {
        perror("Error resizing shared memory");
        exit(EXIT_FAILURE);
    }

    shared_mem = mmap(NULL, BUFFER_SIZE * NUM_LINES, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("Error mapping shared memory");
        exit(EXIT_FAILURE);
    }

    // Создание семафора
    semaphore = sem_open(SEM_NAME, O_CREAT, 0666, 0);
    if (semaphore == SEM_FAILED) {
        perror("Error creating semaphore");
        exit(EXIT_FAILURE);
    }

    // Считывание имени файла
    char filename[BUFFER_SIZE];
    write(STDOUT_FILENO, "Enter filename path: ", 22);
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        fprintf(stderr, "Error reading filename.\n");
        exit(EXIT_FAILURE);
    }

    // Удаление символа '\n' в конце строки
    size_t len = strlen(filename);
    if (len > 0 && filename[len - 1] == '\n') {
        filename[len - 1] = '\0';
    }

    // Запись имени файла в shared memory
    strncpy(shared_mem, filename, BUFFER_SIZE);
    
    // Создание дочернего процесса
    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("Error creating child process");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        execl("./Child", "./Child", NULL);
        perror("Error executing child process");
        exit(EXIT_FAILURE);
    } else {
        sem_post(semaphore); // Сигнализируем дочернему процессу

        // Ожидаем завершения дочернего процесса
        wait(NULL);

        // Чтение результата из shared memory
        for (int i = 0; i < NUM_LINES; i++) {
            if (strlen(shared_mem + i * BUFFER_SIZE) > 0) {
                write(STDOUT_FILENO, shared_mem + i * BUFFER_SIZE, strlen(shared_mem + i * BUFFER_SIZE));
            }
        }

        // Удаление shared memory и семафора
        munmap(shared_mem, BUFFER_SIZE * NUM_LINES);
        shm_unlink(SHM_NAME);
        sem_close(semaphore);
        sem_unlink(SEM_NAME);

        exit(EXIT_SUCCESS);
    }
}
