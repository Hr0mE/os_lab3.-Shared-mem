# ============================================================
#  Лабораторная работа №3
#  Сценарий: сборка модуля управления процессами (Windows)
# ============================================================

# ================== визуальный профиль ==================
$C_FLOW = "`e[38;5;39m"
$C_OK   = "`e[38;5;82m"
$C_WARN = "`e[38;5;214m"
$C_ERR  = "`e[38;5;196m"
$C_DIM  = "`e[2m"
$C_RESET= "`e[0m"

function Flow($msg) { Write-Host "$C_FLOW$msg$C_RESET" }
function Ok($msg)   { Write-Host "$C_OK$msg$C_RESET" }
function Warn($msg) { Write-Host "$C_WARN$msg$C_RESET" }
function Err($msg)  { Write-Host "$C_ERR$msg$C_RESET" }

# ================== окружение ==================
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$ROOT_DIR = Resolve-Path "$ScriptDir\.."
$BUILD_DIR = Join-Path $ROOT_DIR "build"

# ================== фаза инициализации ==================
Flow ">> Инициилизируем рабочее окружение"

if (-Not (Test-Path $BUILD_DIR)) {
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
    Warn ">> Рабочее окружение создано"
} else {
    Flow ">> Рабочее окружение обнаружено"
}

Set-Location $BUILD_DIR

Ok ">> Успешно перешли в рабочий каталог"

# ================== конфигурация ==================
Flow ">> Конфигурация проекта (CMake)"
cmake ..

if ($LASTEXITCODE -ne 0) {
    Err "Ошибка на этапе конфигурации CMake"
    Read-Host "Нажмите Enter, чтобы выйти..."
    exit 1
} else {
    Ok "Конфигурация завершена успешно!"
}

# ================== сборка ==================
Flow ">> Сборка проекта"
cmake --build . --config Release
if ($LASTEXITCODE -ne 0) {
    Err "Ошибка на этапе сборки"
    Read-Host "Нажмите Enter, чтобы выйти..."
    exit 1
}
Ok "Сборка проекта завершена"

# ================== итог ==================
Write-Host ""
Flow "Расположение исполняемого файла:"
Write-Host "  $C_DIM  $BUILD_DIR\bin\os_lab3_main.exe$C_RESET"

Write-Host ""
Flow "Пример запуска:"
Write-Host "  $C_DIM  cd build\bin$C_RESET"
Write-Host "  $C_DIM  .\os_lab3_main.exe$C_RESET"

Write-Host ""
Flow "Запуск нескольких экземпляров:"
Write-Host "  $C_DIM  Start-Process .\os_lab3_main.exe$C_RESET"
Write-Host "  $C_DIM  Start-Process .\os_lab3_main.exe$C_RESET"

Write-Host ""
Flow "Файл логов:"
Write-Host "  $C_DIM  build\bin\logs\process.log$C_RESET"

Write-Host ""
# -------- интерактивный запуск --------
$answer = Read-Host "Хотите запустить тестовый скрипт? [Д/н]"

Flow "Запускаю тестовый скрипт..."
switch ($answer.ToLower()) {
    "" { Start-Process "$BUILD_DIR\bin\os_lab3_main.exe"; Read-Host "Нажмите Enter для выхода..." }
    "д" { Start-Process "$BUILD_DIR\bin\os_lab3_main.exe"; Read-Host "Нажмите Enter для выхода..." }
    "y" { Start-Process "$BUILD_DIR\bin\os_lab3_main.exe"; Read-Host "Нажмите Enter для выхода..." }
    "н" { exit 0 }
    "n" { exit 0 }
    default { 
        Err "Некорректный ввод, завершение программы"
        Read-Host "Нажмите Enter, чтобы выйти..."
        exit 1
    }
}

