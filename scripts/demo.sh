#!/bin/bash

echo ""

cd "$(dirname "$0")/../build/bin"

# Очистка
echo "1. Cleaning up..."
./cleanup_tool
rm -f lab3/logs/process.log

echo ""
echo "2. Starting lab3_main in background..."
echo ""

# Запуск программы
./lab3_main > /dev/null 2>&1 &
PID=$!

echo "Program started with PID: $PID"
sleep 2

echo ""
echo "3. Testing commands..."
echo ""

# Получение текущего значения
echo "   Command: get"
echo "get" | nc localhost 0 2>/dev/null || true
sleep 1

# Установка значения
echo "   Command: set 999"
echo "set 999" | nc localhost 0 2>/dev/null || true
sleep 1

# Получение нового значения  
echo "   Command: get"
echo "get" | nc localhost 0 2>/dev/null || true
sleep 3

echo ""
echo "4. Stopping program..."
kill $PID 2>/dev/null
sleep 1

echo ""
echo "5. Checking logs..."
echo ""
if [ -f lab3/logs/process.log ]; then
    echo "Last 20 lines from log:"
    tail -20 lab3/logs/process.log
else
    echo "No log file found"
fi

echo ""