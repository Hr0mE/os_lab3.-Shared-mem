#include "api/app.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    run_mode_t mode = MODE_NORMAL;
    
    // Определяем режим работы из аргументов
    if (argc > 1) {
        int mode_arg = atoi(argv[1]);
        if (mode_arg == 1) {
            mode = MODE_COPY1;
        } else if (mode_arg == 2) {
            mode = MODE_COPY2;
        }
    }
    
    // Сохраняем путь к программе для запуска копий
    app_set_program_path(argv[0]);
    
    // Инициализация
    if (app_init(mode) != 0) {
        fprintf(stderr, "Failed to initialize application\n");
        return 1;
    }
    
    // Запуск
    int result = app_run();
    
    // Очистка
    app_cleanup();
    
    return result;
}
