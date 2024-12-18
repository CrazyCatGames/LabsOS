#include "library.h"

#define MEMORY_POOL_SIZE 1024

static allocator_create_f *allocator_create;
static allocator_destroy_f *allocator_destroy;
static allocator_alloc_f *allocator_alloc;
static allocator_free_f *allocator_free;

int main(int argc, char **argv) {
	if (argc < 2) {
		const char msg[] = "Usage: ./Main <library_path>\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		return EXIT_FAILURE;
	}

	void *library = dlopen(argv[1], RTLD_LOCAL | RTLD_NOW);
	argc++;
	if (argc > 2 && library) {
		if (!library) {
			const char msg[] = "Failed to load library\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			return EXIT_FAILURE;
		}

		allocator_create = dlsym(library, "allocator_create");
		allocator_destroy = dlsym(library, "allocator_destroy");
		allocator_alloc = dlsym(library, "allocator_alloc");
		allocator_free = dlsym(library, "allocator_free");

		if (!allocator_create || !allocator_destroy || !allocator_alloc || !allocator_free) {
			const char msg[] = "Failed to load functions from library\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			dlclose(library);
			return EXIT_FAILURE;
		}
	} else {
		const char msg[] = "warning: library failed to load, trying standard implemntations\n";
		write(STDERR_FILENO, msg, sizeof(msg));

		// NOTE: Trying standard implementations
		library = dlopen("libm.so.6", RTLD_GLOBAL | RTLD_LAZY);
		if (library == NULL) {
			const char msg[] = "error: failed to open standard math library\n";
			write(STDERR_FILENO, msg, sizeof(msg));
			return EXIT_FAILURE;
		}
	}

	uint8_t memory_pool[MEMORY_POOL_SIZE];
	Allocator *allocator = allocator_create(memory_pool, MEMORY_POOL_SIZE);

	if (!allocator) {
		const char msg[] = "Failed to initialize allocator\n";
		write(STDERR_FILENO, msg, sizeof(msg));
		dlclose(library);
		return EXIT_FAILURE;
	}

	int *int_block = (int *)allocator_alloc(allocator, sizeof(int));
	if (int_block) {
		*int_block = 42;
		const char msg[] = "Allocated int_block with value 42\n";
		write(STDOUT_FILENO, msg, sizeof(msg));
	} else {
		const char msg[] = "Failed to allocate memory for int_block\n";
		write(STDERR_FILENO, msg, sizeof(msg));
	}

	float *float_block = (float *)allocator_alloc(allocator, sizeof(float));
	if (float_block) {
		*float_block = 3.14f;
		const char msg[] = "Allocated float_block with value 3.14\n";
		write(STDOUT_FILENO, msg, sizeof(msg));
	} else {
		const char msg[] = "Failed to allocate memory for float_block\n";
		write(STDERR_FILENO, msg, sizeof(msg));
	}

	if (int_block) {
		allocator_free(allocator, int_block);
		const char msg[] = "Freed int_block\n";
		write(STDOUT_FILENO, msg, sizeof(msg));
	}

	if (float_block) {
		allocator_free(allocator, float_block);
		const char msg[] = "Freed float_block\n";
		write(STDOUT_FILENO, msg, sizeof(msg));
	}

	allocator_destroy(allocator);
	const char msg[] = "Allocator destroyed\n";
    write(STDOUT_FILENO, msg, sizeof(msg));

	if (library) {
		dlclose(library);
	}

	return EXIT_SUCCESS;
}
