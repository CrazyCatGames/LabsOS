#include "library.h"
#include <stdint.h>
#include <stddef.h>

// Структура блока памяти
struct Block {
    size_t size;        // Размер блока
    Block *next;        // Указатель на следующий свободный блок
};

// Структура аллокатора
struct Allocator {
    Block *free_list;   // Список свободных блоков
    uint8_t *memory;    // Память, выделенная для аллокатора
    size_t total_size;  // Общий размер памяти
};

// Функция для создания аллокатора
Allocator* allocator_create(void *const memory, const size_t size) {
    if (!memory || size < sizeof(Block)) {
        return NULL; // Не хватает памяти для аллокатора
    }

    Allocator *allocator = (Allocator*) memory;
    allocator->memory = (uint8_t*) memory + sizeof(Allocator); // Отступаем от памяти для аллокатора
    allocator->total_size = size - sizeof(Allocator);
    allocator->free_list = (Block*) allocator->memory;

    // Инициализация списка свободных блоков
    allocator->free_list->size = allocator->total_size;
    allocator->free_list->next = NULL;

    return allocator;
}

// Функция для уничтожения аллокатора
void allocator_destroy(Allocator *const allocator) {
    if (allocator) {
        memset(allocator, 0, allocator->total_size);
    }
}

// Функция для выделения памяти
void* allocator_alloc(Allocator *const allocator, const size_t size) {
    if (size == 0) return NULL;

    Block **prev = &allocator->free_list;
    Block *current = allocator->free_list;

    // Ищем первый подходящий свободный блок
    while (current) {
        if (current->size >= size) {
            // Найден подходящий блок
            *prev = current->next; // Убираем блок из списка свободных

            // Если блок больше чем необходимо, делим его на два
            if (current->size > size + sizeof(Block)) {
                // Разделяем блок
                Block *remaining = (Block*)((uint8_t*)current + size + sizeof(Block));
                remaining->size = current->size - size - sizeof(Block);
                remaining->next = current->next;

                // Обновляем размер текущего блока
                current->size = size;
                current->next = remaining;
            }

            return (void*)((uint8_t*)current + sizeof(Block));
        }
        prev = &current->next;
        current = current->next;
    }

    return NULL; // Нет достаточно большого свободного блока
}

// Функция для освобождения памяти
void allocator_free(Allocator *const allocator, void *const memory) {
    if (!memory) return;

    // Получаем указатель на блок
    Block *block = (Block*)((uint8_t*)memory - sizeof(Block));

    // Вставляем блок обратно в список свободных
    block->next = allocator->free_list;
    allocator->free_list = block;
}
