#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#define DECK_NUM 52

int matches = 0;
pthread_mutex_t m;

void PrintHelper(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buffer[BUFSIZ];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	if (write(STDOUT_FILENO, buffer, strlen(buffer)) == -1) {
		exit(EXIT_FAILURE);
	}
	va_end(args);
}

void *ThreadFunction(void *args) {
	size_t round = *(size_t *)args;
    unsigned int seed = ((size_t *)args)[1];
	int local = 0;
	int deck[DECK_NUM];
	for (int j = 0; j < DECK_NUM; j++) {
		deck[j] = j;
	}

	for (size_t i = 0; i < round; i++) {
		for (int j = DECK_NUM - 1; j > 0; j--) {
			int k = rand_r(&seed) % (j + 1);

			int temp = deck[j];
			deck[j] = deck[k];
			deck[k] = temp;
		}

		if (deck[0] % 4 == deck[1] % 4) {
			++local;
		}
	}
    pthread_mutex_lock(&m);
	matches += local;
    pthread_mutex_unlock(&m);
	return NULL;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		PrintHelper("Input error. Use: %s <threads> <rounds>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	size_t threads_num = atol(argv[1]);
	size_t rounds = atol(argv[2]);
	size_t rounds_for_thread = rounds / threads_num;
	pthread_t threads[threads_num];

	pthread_mutex_init(&m, NULL);

	for (size_t i = 0; i < threads_num; i++) {
		if (pthread_create(&threads[i], NULL, ThreadFunction, &rounds_for_thread) != 0) {
			PrintHelper("Error. Thread %zu not created\n", i);
			exit(EXIT_FAILURE);
		}
	}

	for (size_t i = 0; i < threads_num; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			PrintHelper("Error. Thread not joined\n");
			exit(EXIT_FAILURE);
		}
	}
    
	pthread_mutex_destroy(&m);

	PrintHelper("==== %d ====\n", matches);

	double probability = (double)matches / (double)rounds;
	PrintHelper("== %.6f ==\n", probability);

	return EXIT_SUCCESS;
}
