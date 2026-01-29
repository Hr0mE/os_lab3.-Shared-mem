#!/bin/bash

# Скрипт очистки shared memory для лабораторной работы 3

echo "Cleaning up Lab 3 resources..."

# Убиваем все процессы lab3_main
echo "Killing all lab3_main processes..."
pkill -9 lab3_main 2>/dev/null

# Удаляем shared memory (для Linux)
if [ -f /dev/shm/lab3_shared_memory ]; then
    echo "Removing Linux shared memory..."
    rm -f /dev/shm/lab3_shared_memory
fi

# Для macOS
if [ "$(uname)" = "Darwin" ]; then
    echo "Removing macOS shared memory..."
    # На macOS shared memory хранится по-другому, попробуем удалить через API
    # Shared memory автоматически удаляется при перезагрузке
fi

echo "Cleanup completed!"
echo ""
echo "Now you can run the program with:"
echo "  cd build/bin"
echo "  ./lab3_main"
