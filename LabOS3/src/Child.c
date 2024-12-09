#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define SHM_NAME "/shared_memory"
#define SEM_NAME "/sync_semaphore"
#define BUFFER_SIZE 1024
#define NUM_LINES 100

void HandleError(const char *message) {
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
}

int main() {
    int shm_fd;
    char *shared_mem;
    sem_t *semaphore;

    // Подключение к существующей разделяемой памяти
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        HandleError("Error connecting to shared memory.\n");
    }

    shared_mem = mmap(NULL, BUFFER_SIZE * NUM_LINES, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        HandleError("Error mapping shared memory.\n");
    }

    // Подключение к существующему семафору
    semaphore = sem_open(SEM_NAME, 0);
    if (semaphore == SEM_FAILED) {
        HandleError("Error connecting to semaphore.\n");
    }

    // Ожидание данных от родительского процесса
    sem_wait(semaphore);

    // Чтение имени файла из shared memory
    char filename[BUFFER_SIZE];
    strncpy(filename, shared_mem, BUFFER_SIZE);

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        strncpy(shared_mem, "Error: Unable to open file.\n", BUFFER_SIZE);
        sem_post(semaphore);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    float num_first, num_next;
    char *current;
    int line_number = 0;

    while ((bytesRead = read(file, buffer, BUFFER_SIZE)) > 0) {
        current = buffer;

        // Чтение первого числа
        while (current < buffer + bytesRead) {
            while (*current == ' ' || *current == '\t') current++;
            if (*current == '\n') {
                current++;
                continue;
            }

            char *endptr;
            num_first = strtof(current, &endptr);
            if (endptr == current) {
                strncpy(shared_mem + line_number * BUFFER_SIZE, "Error: Non-numeric value encountered.\n", BUFFER_SIZE);
            }
            current = endptr;

            // Чтение и деление на последующие числа
            while (current < buffer + bytesRead && *current != '\n') {
                while (*current == ' ' || *current == '\t') current++;
                if (*current == '\n') break;

                num_next = strtof(current, &endptr);
                if (endptr == current || num_next == 0.0) {
                    strncpy(shared_mem + line_number * BUFFER_SIZE, "Error: Division by zero or invalid input.\n", BUFFER_SIZE);
                    sem_post(semaphore);
                    exit(EXIT_FAILURE);
                }
                num_first /= num_next;
                current = endptr;
            }

            // Запись результата в shared memory для каждой строки
            char result[BUFFER_SIZE];
            int result_len = snprintf(result, BUFFER_SIZE, "Result: %.6f\n", num_first);
            strncpy(shared_mem + line_number * BUFFER_SIZE, result, result_len + 1);
            line_number++;

            if (line_number >= NUM_LINES) break;
        }
    }

    if (bytesRead == -1) {
        strncpy(shared_mem, "Error reading file.\n", BUFFER_SIZE);
        sem_post(semaphore);
        exit(EXIT_FAILURE);
    }

    close(file);
    sem_post(semaphore);
    exit(EXIT_SUCCESS);
}