#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdio.h>
#include <stdbool.h>

// Имя межпроцессного мьютекса
//#define SHARED_MUTEX_NAME "lab3_shared_mutex"

// Получить текущий PID
int platform_get_pid(void);

// Получить текущее время в миллисекундах
long long platform_get_time_ms(void);

// Форматировать время для лога
void platform_format_time(char *buffer, size_t size);

// Спать миллисекунды
void platform_sleep_ms(int ms);

// Запустить копию программы с аргументом
int platform_spawn_copy(const char *program_path, int copy_type);

// Проверить, завершился ли дочерний процесс
bool platform_check_process_finished(int pid);

// Создать/открыть лог файл
FILE *platform_open_log(const char *filename, bool append);

// Создать мьютекс
void *platform_create_mutex(void);

// Уничтожить мьютекс
void platform_destroy_mutex(void *mutex);

// Захватить мьютекс
void platform_lock_mutex(void *mutex);

// Освободить мьютекс
void platform_unlock_mutex(void *mutex);

// Создать shared memory
void *platform_create_shared_memory(const char *name, size_t size);

// Открыть существующую shared memory
void *platform_open_shared_memory(const char *name, size_t size);

// Закрыть shared memory
void platform_close_shared_memory(void *ptr, size_t size);

// Удалить shared memory
void platform_unlink_shared_memory(const char *name);

#endif /* PLATFORM_H */
