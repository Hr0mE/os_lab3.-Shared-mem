#include "api/app.h"
#include "api/shared.h"
#include "internal/platform.h"
#include "internal/platform_fs.h"
#include "internal/platform_io.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

/* ================== визуальный профиль ================== */
#define C_FLOW "\033[38;5;39m"     /* основной поток */
#define C_OK   "\033[38;5;82m"     /* успех */
#define C_WARN "\033[38;5;214m"    /* предупреждение */
#define C_ERR  "\033[38;5;196m"    /* ошибка */
#define C_CMD "\033[97m"  /* белый цвет для команд */
#define C_RESET "\033[0m"

#define flow(msg, ...)  printf(C_FLOW msg C_RESET "\n", ##__VA_ARGS__)
#define ok(msg, ...)    printf(C_OK   msg C_RESET "\n", ##__VA_ARGS__)
#define warn(msg, ...)  printf(C_WARN msg C_RESET "\n", ##__VA_ARGS__)
#define err(msg, ...)   fprintf(stderr, C_ERR msg C_RESET "\n", ##__VA_ARGS__)

/* ================== глобальные переменные ================== */
#define LOG_FILE "logs/process.log"
#define TIMER_300MS 300
#define TIMER_1SEC 1000
#define TIMER_3SEC 3000

static FILE* g_log_file = NULL;
static run_mode_t g_mode = MODE_NORMAL;
static int g_my_pid = 0;
static bool g_is_master = false;
static long long g_start_time = 0;
static long long g_last_300ms = 0;
static long long g_last_1sec = 0;
static long long g_last_3sec = 0;
static int g_copy1_pid = 0;
static int g_copy2_pid = 0;
static char g_program_path[1024] = {0};
static bool g_running = true;

/* ================== логирование ================== */
void app_log(const char* format, ...) {
    if (!g_log_file) return;

    char time_buffer[64];
    platform_format_time(time_buffer, sizeof(time_buffer));

    fprintf(g_log_file, "[%s] [PID:%d] ", time_buffer, g_my_pid);

    va_list args;
    va_start(args, format);
    vfprintf(g_log_file, format, args);
    va_end(args);

    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

/* ================== инициализация приложения ================== */
int app_init(run_mode_t mode) {
    g_mode = mode;
    g_my_pid = platform_get_pid();
    g_start_time = platform_get_time_ms();

    /* Инициализация shared memory */
    if (shared_init() != 0) {
        err("Не удалось инициализировать shared memory");
        return -1;
    }

    /* Создание директории логов, если не существует */
    if (ensure_logs_dir("logs") != 0) {
        err("Не удалось создать директорию logs");
        return -1;
    }

    /* Открытие файла логов */
    g_log_file = platform_open_log(LOG_FILE, true);
    if (!g_log_file) {
        err("Не удалось открыть лог-файл: path='%s', errno=%d (%s)",
            LOG_FILE, errno, strerror(errno));
        shared_cleanup();
        return -1;
    }

    /* Инициализация таймеров */
    g_last_300ms = g_start_time;
    g_last_1sec = g_start_time;
    g_last_3sec = g_start_time;

    /* Логируем старт */
    app_log("Процесс запущен (mode=%d)", mode);

    /* Определяем мастер-процесс (только для нормального режима) */
    if (g_mode == MODE_NORMAL) {
        g_is_master = shared_try_become_master(g_my_pid);
        if (g_is_master) {
            app_log("Стал MASTER процесс");
            flow("Процесс %d: MASTER mode", g_my_pid);
        } else {
            app_log("Запущен как SLAVE процесс (master=%d)", shared_get_master_pid());
            flow("Процесс %d: SLAVE mode (master=%d)", g_my_pid, shared_get_master_pid());
        }
    }

    return 0;
}

/* ================== завершение приложения ================== */
void app_cleanup(void) {
    app_log("Процесс завершает работу");

    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }

    shared_cleanup();
}

/* ================== обработка копий ================== */
static void handle_copy1_mode(void) {
    app_log("Copy1 started");
    shared_set_copy_status(1, true);

    platform_sleep_ms(100); /* небольшая задержка */

    shared_add_counter(10);
    int64_t counter = shared_get_counter();
    app_log("Copy1: counter += 10, новое значение = %lld", (long long)counter);

    shared_set_copy_status(1, false);
    app_log("Copy1 exiting");
}

static void handle_copy2_mode(void) {
    app_log("Copy2 started");
    shared_set_copy_status(2, true);

    platform_sleep_ms(100);

    shared_multiply_counter(2);
    int64_t counter = shared_get_counter();
    app_log("Copy2: counter *= 2, новое значение = %lld", (long long)counter);

    platform_sleep_ms(2000);

    shared_divide_counter(2);
    counter = shared_get_counter();
    app_log("Copy2: counter /= 2, новое значение = %lld", (long long)counter);

    shared_set_copy_status(2, false);
    app_log("Copy2 exiting");
}

/* ================== обработчики таймеров ================== */
static void timer_300ms_handler(void) {
    shared_add_counter(1);
}

static void timer_1sec_handler(void) {
    if (!g_is_master) return;

    int64_t counter = shared_get_counter();
    app_log("Timer 1s: counter = %lld", (long long)counter);
}

static void timer_3sec_handler(void) {
    if (!g_is_master) return;

    bool copy1_busy = shared_get_copy_status(1);
    bool copy2_busy = shared_get_copy_status(2);

    if (g_copy1_pid > 0 && platform_check_process_finished(g_copy1_pid)) {
        g_copy1_pid = 0;
        copy1_busy = false;
        shared_set_copy_status(1, false);
    }

    if (g_copy2_pid > 0 && platform_check_process_finished(g_copy2_pid)) {
        g_copy2_pid = 0;
        copy2_busy = false;
        shared_set_copy_status(2, false);
    }

    /* Запуск копии 1 */
    if (!copy1_busy) {
        g_copy1_pid = platform_spawn_copy(g_program_path, MODE_COPY1);
        if (g_copy1_pid > 0)
            app_log("Spawned Copy1 with PID %d", g_copy1_pid);
        else
            app_log("Failed to spawn Copy1");
    } else {
        app_log("Copy1 еще работает, пропускаем запуск");
    }

    /* Запуск копии 2 */
    if (!copy2_busy) {
        g_copy2_pid = platform_spawn_copy(g_program_path, MODE_COPY2);
        if (g_copy2_pid > 0)
            app_log("Spawned Copy2 with PID %d", g_copy2_pid);
        else
            app_log("Failed to spawn Copy2");
    } else {
        app_log("Copy2 еще работает, пропускаем запуск");
    }
}

/* ================== ввод пользователя ================== */
void app_handle_input(void) {
    int ready = platform_poll_stdin(0); /* неблокирующий режим */
    if (ready <= 0) return;

    char buffer[256];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) == 0) return;

        if (strcmp(buffer, "quit") == 0 || strcmp(buffer, "exit") == 0 || strcmp(buffer, "q") == 0) {
            flow("Завершение программы по команде пользователя...");
            g_running = false;
        } else if (strcmp(buffer, "get") == 0) {
            int64_t value = shared_get_counter();
            flow("Counter = %lld", (long long)value);
        } else if (strncmp(buffer, "set ", 4) == 0) {
            long long value = atoll(buffer + 4);
            shared_set_counter(value);
            flow("Counter установлен в %lld", value);
            app_log("Пользователь установил counter = %lld", value);
        } else {
            warn("Неизвестная команда. Доступные: set <value>, get, quit");
        }
    }
}

/* ================== основной цикл ================== */
int app_run(void) {
    if (g_mode == MODE_COPY1) {
        handle_copy1_mode();
        return 0;
    } else if (g_mode == MODE_COPY2) {
        handle_copy2_mode();
        return 0;
    }

   
    flow("\n=== Lab 3 Process Manager ===");
    flow("PID: %d, Mode: %s", g_my_pid, g_is_master ? "MASTER" : "SLAVE");
    flow("Commands:\n  " C_CMD "set <value>" C_FLOW " - задать значение счетчика\n"
         "  " C_CMD "get" C_FLOW " - получить текущее значение\n"
         "  " C_CMD "quit" C_FLOW " - выйти из программы");
    flow("==============================\n");


    while (g_running) {
        long long current_time = platform_get_time_ms();

        if (current_time - g_last_300ms >= TIMER_300MS) {
            timer_300ms_handler();
            g_last_300ms = current_time;
        }

        if (current_time - g_last_1sec >= TIMER_1SEC) {
            timer_1sec_handler();
            g_last_1sec = current_time;
        }

        if (current_time - g_last_3sec >= TIMER_3SEC) {
            timer_3sec_handler();
            g_last_3sec = current_time;
        }

        app_handle_input();

        platform_sleep_ms(50); /* короткая пауза, чтобы CPU не загружать */
    }

    return 0;
}

/* ================== установка пути программы ================== */
void app_set_program_path(const char* path) {
    strncpy(g_program_path, path, sizeof(g_program_path) - 1);
    g_program_path[sizeof(g_program_path) - 1] = '\0';
}

