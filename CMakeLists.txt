cmake_minimum_required(VERSION 3.10)
project(OSLab3ProcessManager C)

set(CMAKE_C_STANDARD 99)

# Создаем директорию для логов
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/logs")

# Включаем директории заголовочных файлов
include_directories(include)

# Исходные файлы
set(SOURCES
    src/main.c
    src/app.c
    src/shared.c
    src/platform.c
    src/platform_fs.c
    src/platform_io.c
)


# Создаем исполняемый файл
add_executable(os_lab3_main ${SOURCES})

# Утилита очистки shared memory
add_executable(cleanup_tool
    src/cleanup_tool.c
    src/shared.c
    src/platform.c
    src/platform_fs.c
    src/platform_io.c
)

# Линковка библиотек для POSIX систем
if(UNIX AND NOT APPLE)
    target_link_libraries(os_lab3_main pthread rt)
    target_link_libraries(cleanup_tool pthread rt)
elseif(UNIX)
    target_link_libraries(os_lab3_main pthread)
    target_link_libraries(cleanup_tool pthread)
endif()

# Установка опций компиляции
if(MSVC)
    target_compile_options(os_lab3_main PRIVATE /W4)
    target_compile_options(cleanup_tool PRIVATE /W4)
else()
    target_compile_options(os_lab3_main PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(cleanup_tool PRIVATE -Wall -Wextra -pedantic)
endif()

# Установка выходной директории
set_target_properties(os_lab3_main cleanup_tool PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Копирование директории логов в build
add_custom_command(TARGET os_lab3_main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    "${CMAKE_BINARY_DIR}/bin/logs"
)
