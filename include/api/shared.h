#ifndef SHARED_H
#define SHARED_H

#include <stdint.h>
#include <stdbool.h>

// Имя разделяемой памяти
#define SHARED_MEMORY_NAME "lab3_shared_memory"
#define SHARED_MEMORY_SIZE sizeof(shared_data_t)

// Структура разделяемых данных
typedef struct
{
    int64_t counter;    // Счетчик
    int master_pid;     // PID главного процесса
    bool copy1_running; // Флаг выполнения копии 1
    bool copy2_running; // Флаг выполнения копии 2
} shared_data_t;

// Инициализация разделяемой памяти
int shared_init(void);

// Освобождение разделяемой памяти
void shared_cleanup(void);

// Получить указатель на разделяемые данные
shared_data_t *shared_get_data(void);

// Установить значение счетчика
void shared_set_counter(int64_t value);

// Получить значение счетчика
int64_t shared_get_counter(void);

// Добавить к счетчику
void shared_add_counter(int64_t delta);

// Умножить счетчик
void shared_multiply_counter(int multiplier);

// Делить счетчик
void shared_divide_counter(int divisor);

// Установить/проверить флаг главного процесса
bool shared_try_become_master(int pid);
int shared_get_master_pid(void);

// Установить/получить статус копий
void shared_set_copy_status(int copy_num, bool running);
bool shared_get_copy_status(int copy_num);

#endif /* SHARED_H */
