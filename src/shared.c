#include "api/shared.h"
#include "internal/platform.h"
#include <string.h>
#include <stdio.h>

static shared_data_t* g_shared_data = NULL;
static void* g_mutex = NULL;

int shared_init(void) {
    // Пробуем открыть существующую shared memory
    g_shared_data = (shared_data_t*)platform_open_shared_memory(
        SHARED_MEMORY_NAME, SHARED_MEMORY_SIZE);
    
    bool is_new = false;

    if (g_shared_data == NULL) {
        // Создаем новую
        g_shared_data = (shared_data_t*)platform_create_shared_memory(
            SHARED_MEMORY_NAME, SHARED_MEMORY_SIZE);
        
        if (g_shared_data == NULL) {
            fprintf(stderr, "Failed to create shared memory\n");
            return -1;
        }
        
        is_new = true;
        // Инициализируем данные
        memset(g_shared_data, 0, sizeof(shared_data_t));
        g_shared_data->counter = 0;
        g_shared_data->master_pid = 0;
        g_shared_data->copy1_running = false;
        g_shared_data->copy2_running = false;
    }
    
    // Создаем мьютекс (каждый процесс имеет свой)
    g_mutex = platform_create_mutex();
    if (g_mutex == NULL) {
        fprintf(stderr, "Failed to create mutex\n");
        return -1;
    }
    
    return 0;
}

void shared_cleanup(void) {
    if (g_mutex != NULL) {
        platform_destroy_mutex(g_mutex);
        g_mutex = NULL;
    }
    
    if (g_shared_data != NULL) {
        platform_close_shared_memory(g_shared_data, SHARED_MEMORY_SIZE);
        g_shared_data = NULL;
    }
}

shared_data_t* shared_get_data(void) {
    return g_shared_data;
}

void shared_set_counter(int64_t value) {
    if (g_shared_data == NULL) return;
    
    platform_lock_mutex(g_mutex);
    g_shared_data->counter = value;
    platform_unlock_mutex(g_mutex);
}

int64_t shared_get_counter(void) {
    if (g_shared_data == NULL) return 0;
    
    platform_lock_mutex(g_mutex);
    int64_t value = g_shared_data->counter;
    platform_unlock_mutex(g_mutex);
    
    return value;
}

void shared_add_counter(int64_t delta) {
    if (g_shared_data == NULL) return;
    
    platform_lock_mutex(g_mutex);
    g_shared_data->counter += delta;
    platform_unlock_mutex(g_mutex);
}

void shared_multiply_counter(int multiplier) {
    if (g_shared_data == NULL) return;
    
    platform_lock_mutex(g_mutex);
    g_shared_data->counter *= multiplier;
    platform_unlock_mutex(g_mutex);
}

void shared_divide_counter(int divisor) {
    if (g_shared_data == NULL || divisor == 0) return;
    
    platform_lock_mutex(g_mutex);
    g_shared_data->counter /= divisor;
    platform_unlock_mutex(g_mutex);
}

bool shared_try_become_master(int pid) {
    if (g_shared_data == NULL) return false;
    
    platform_lock_mutex(g_mutex);
    
    if (g_shared_data->master_pid == 0) {
        // Никто еще не мастер
        g_shared_data->master_pid = pid;
        platform_unlock_mutex(g_mutex);
        return true;
    }
    
    if (g_shared_data->master_pid == pid) {
        // Мы уже мастер
        platform_unlock_mutex(g_mutex);
        return true;
    }
    
    // Кто-то другой уже мастер
    platform_unlock_mutex(g_mutex);
    return false;
}

int shared_get_master_pid(void) {
    if (g_shared_data == NULL) return 0;
    
    platform_lock_mutex(g_mutex);
    int pid = g_shared_data->master_pid;
    platform_unlock_mutex(g_mutex);
    
    return pid;
}

void shared_set_copy_status(int copy_num, bool running) {
    if (g_shared_data == NULL) return;
    
    platform_lock_mutex(g_mutex);
    if (copy_num == 1) {
        g_shared_data->copy1_running = running;
    } else if (copy_num == 2) {
        g_shared_data->copy2_running = running;
    }
    platform_unlock_mutex(g_mutex);
}

bool shared_get_copy_status(int copy_num) {
    if (g_shared_data == NULL) return false;
    
    platform_lock_mutex(g_mutex);
    bool status = false;
    if (copy_num == 1) {
        status = g_shared_data->copy1_running;
    } else if (copy_num == 2) {
        status = g_shared_data->copy2_running;
    }
    platform_unlock_mutex(g_mutex);
    
    return status;
}
