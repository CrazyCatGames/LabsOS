#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void HandleError(const char *message) {
	write(STDERR_FILENO, message, strlen(message));
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		HandleError("Error: need to specify the path to the file.\n");
	}

	int file = open(argv[1], O_RDONLY);
	if (file == -1) {
		HandleError("Error opening file.\n");
	}

	// Redefining the standard input to the file
	dup2(file, STDIN_FILENO);
	close(file);  // Close original descriptor

	char buffer[BUFFER_SIZE];
	ssize_t bytesRead;
	float num_first, num_next;
	char *current;
	char output[BUFFER_SIZE];
	int output_len;
    int flag = 0;
	int divisionByZeroError = 0;

	// Read strings
	while ((bytesRead = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
		current = buffer;

		// Read first number
		while (current < buffer + bytesRead) {
			while (*current == ' ' || *current == '\t') {
				current++;
			}
			if (*current == '\n') {
				current++;
				continue;
			}

			if (current >= buffer + bytesRead) {
				break;
			}

			char *endptr;
			num_first = strtof(current, &endptr);

			if (endptr == current) {
				output_len = snprintf(output, BUFFER_SIZE, "Error: Non-numeric value encountered.\n");
				write(STDOUT_FILENO, output, output_len);
				while (*current != '\n' && current < buffer + bytesRead) {
					current++;
				}
				if (*current == '\n') {
					current++;
				}
				continue;
			}

			current = endptr;

			// Read other numbers
			while (1) {
				while (*current == ' ' || *current == '\t') {
					current++;
				}

				if (*current == '\n' || current >= buffer + bytesRead) {
					break;
				}

				num_next = strtof(current, &endptr);
				flag = 0;
				if (endptr == current) {
					output_len = snprintf(output, BUFFER_SIZE, "Error: Non-numeric value encountered.\n");
					write(STDOUT_FILENO, output, output_len);
					while (*current != '\n' && current < buffer + bytesRead) {
						current++;
					}
					if (*current == '\n') {
                        flag = 1;
						continue;
					}
				}

				if (num_next == 0.0 || current == NULL || current == buffer) {
					output_len = snprintf(output, BUFFER_SIZE, "Division by zero.\n");
					write(STDOUT_FILENO, output, output_len);
					exit(EXIT_FAILURE);
				}

				num_first /= num_next;
				current = endptr;
			}
			if (flag == 0) {
				output_len = snprintf(output, BUFFER_SIZE, "Result: %.6f\n", num_first);  // Get answer string
				write(STDOUT_FILENO, output, output_len);
			}else{
                NULL;
            }
			while (*current != '\n' && current < buffer + bytesRead) {
				current++;
			}

			if (*current == '\n') {
				current++;
			}
		}
	}

	if (bytesRead == -1) {
		HandleError("Error reading file.\n");
	}

	exit(EXIT_SUCCESS);
}
