@echo off
REM Скрипт сборки для лабораторной работы 3 (Windows)

echo ==========================================
echo Building Lab 3 - Process Manager
echo ==========================================

REM Создаем директорию для сборки
if not exist build mkdir build
cd build

REM Запускаем CMake
echo Running CMake...
cmake ..

if %ERRORLEVEL% NEQ 0 (
    echo CMake failed!
    exit /b 1
)

REM Компилируем проект
echo Building project...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo ==========================================
echo Build completed successfully!
echo ==========================================
echo.
echo Executable is in: build\bin\Release\
echo.
echo To run:
echo   cd build\bin\Release
echo   lab3_main.exe
echo.
echo To run multiple instances:
echo   start lab3_main.exe
echo   start lab3_main.exe
echo.
echo Check logs in: build\bin\Release\lab3\logs\process.log
echo.

cd ..
