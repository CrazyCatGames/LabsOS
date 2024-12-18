#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef struct Allocator Allocator;
typedef struct Block Block;

// инициализация структуры аллокатора
typedef Allocator *allocator_create_f(void *const memory, const size_t size);
// деинициализация структуры аллокатора
typedef void allocator_destroy_f(Allocator *const allocator);
// выделение памяти аллокатором памяти размера size
typedef void *allocator_alloc_f(Allocator *const allocator, const size_t size);
// возвращает выделенную память аллокатору
typedef void allocator_free_f(Allocator *const allocator, void *const memory);

#endif // ALLOCATOR_H
