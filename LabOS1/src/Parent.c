#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int main() {
    int pipe1[2];
    pid_t child_pid;
    char buffer;
    char filename[BUFFER_SIZE];
    ssize_t bytesRead;

    write(STDOUT_FILENO, "Enter filename path: ", 22);
    bytesRead = read(STDIN_FILENO, filename, sizeof(filename));
    if (bytesRead <= 0) {
        write(STDERR_FILENO, "Error reading the file name\n", 29);
        exit(EXIT_FAILURE);
    }

    if (filename[bytesRead - 1] == '\n') {
        filename[bytesRead - 1] = '\0';
    }

    if (pipe(pipe1) == -1) {
        write(STDERR_FILENO, "Pipe creation error\n", 21);
        exit(EXIT_FAILURE);
    }

    child_pid = fork();
    if (child_pid == -1) {
        write(STDERR_FILENO, "Process creation error\n", 24);
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        close(pipe1[0]);

        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[1]);
        
        execl("./child", "", filename, NULL);
        write(STDERR_FILENO, "Error starting a child process\n", 32);
        exit(EXIT_FAILURE);
    } else {
        close(pipe1[1]);

        // Read data from pipe and print
        while (read(pipe1[0], &buffer, 1) > 0) {
            write(STDOUT_FILENO, &buffer, 1);
        }
        close(pipe1[0]);

        wait(NULL);  // Waiting until finish child process
        exit(EXIT_SUCCESS);
    }
    
    return 0;
}
