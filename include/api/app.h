#ifndef APP_H
#define APP_H

#include <stdbool.h>

// Типы запуска программы
typedef enum
{
    MODE_NORMAL = 0, // Обычный режим
    MODE_COPY1 = 1,  // Копия 1 (+10)
    MODE_COPY2 = 2   // Копия 2 (*2, потом /2)
} run_mode_t;

// Инициализация приложения
int app_init(run_mode_t mode);

// Основной цикл приложения
int app_run(void);

// Очистка ресурсов
void app_cleanup(void);

// Обработка пользовательского ввода
void app_handle_input(void);

// Логирование
void app_log(const char *format, ...);

// Установить путь к исполняемому файлу
void app_set_program_path(const char *path);

#endif /* APP_H */
