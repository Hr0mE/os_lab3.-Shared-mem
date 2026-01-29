#include "internal/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #define PATH_SEPARATOR "\\"
#else
    #include <unistd.h>
    #include <sys/time.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <pthread.h>
    #include <errno.h>
    #define PATH_SEPARATOR "/"
#endif

int platform_get_pid(void) {
#ifdef _WIN32
    return (int)GetCurrentProcessId();
#else
    return (int)getpid();
#endif
}

long long platform_get_time_ms(void) {
#ifdef _WIN32
    return (long long)GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000 + (long long)(tv.tv_usec) / 1000;
#endif
}

void platform_format_time(char* buffer, size_t size) {
    time_t now;
    struct tm* tm_info;
    
    time(&now);
    tm_info = localtime(&now);
    
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(buffer + strlen(buffer), size - strlen(buffer), ".%03d", (int)(tv.tv_usec / 1000));
#endif
}

void platform_sleep_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

int platform_spawn_copy(const char* program_path, int copy_type) {
    char arg_buffer[32];
    snprintf(arg_buffer, sizeof(arg_buffer), "%d", copy_type);
    
#ifdef _WIN32
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    
    char cmdline[1024];
    snprintf(cmdline, sizeof(cmdline), "\"%s\" %s", program_path, arg_buffer);
    
    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        return -1;
    }
    
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return (int)pi.dwProcessId;
#else
    pid_t pid = fork();
    
    if (pid < 0) {
        return -1;
    }
    
    if (pid == 0) {
        // Дочерний процесс
        execl(program_path, program_path, arg_buffer, (char*)NULL);
        exit(1); // Если execl не удался
    }
    
    return (int)pid;
#endif
}

bool platform_check_process_finished(int pid) {
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, (DWORD)pid);
    if (hProcess == NULL) {
        return true; // Процесс не существует
    }
    
    DWORD exit_code;
    GetExitCodeProcess(hProcess, &exit_code);
    CloseHandle(hProcess);
    
    return (exit_code != STILL_ACTIVE);
#else
    int status;
    pid_t result = waitpid((pid_t)pid, &status, WNOHANG);
    return (result != 0); // 0 = еще работает, >0 = завершился, -1 = ошибка
#endif
}

FILE* platform_open_log(const char* filename, bool append) {
    return fopen(filename, append ? "a" : "w");
}

void* platform_create_mutex(void) {
#ifdef _WIN32
    HANDLE mutex = CreateMutexA(NULL, FALSE, NULL);
    return (void*)mutex;
#else
    pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
    return (void*)mutex;
#endif
}

void platform_destroy_mutex(void* mutex) {
    if (mutex == NULL) return;
    
#ifdef _WIN32
    CloseHandle((HANDLE)mutex);
#else
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
    free(mutex);
#endif
}

void platform_lock_mutex(void* mutex) {
    if (mutex == NULL) return;
    
#ifdef _WIN32
    WaitForSingleObject((HANDLE)mutex, INFINITE);
#else
    pthread_mutex_lock((pthread_mutex_t*)mutex);
#endif
}

void platform_unlock_mutex(void* mutex) {
    if (mutex == NULL) return;
    
#ifdef _WIN32
    ReleaseMutex((HANDLE)mutex);
#else
    pthread_mutex_unlock((pthread_mutex_t*)mutex);
#endif
}

void* platform_create_shared_memory(const char* name, size_t size) {
#ifdef _WIN32
    HANDLE hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        (DWORD)size,
        name);
    
    if (hMapFile == NULL) {
        return NULL;
    }
    
    void* ptr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
    return ptr;
#else
    int fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        return NULL;
    }
    
    if (ftruncate(fd, size) < 0) {
        close(fd);
        return NULL;
    }
    
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    
    return ptr;
#endif
}

void* platform_open_shared_memory(const char* name, size_t size) {
#ifdef _WIN32
    HANDLE hMapFile = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (hMapFile == NULL) {
        return NULL;
    }
    
    void* ptr = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
    return ptr;
#else
    int fd = shm_open(name, O_RDWR, 0666);
    if (fd < 0) {
        return NULL;
    }
    
    void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    
    return ptr;
#endif
}

void platform_close_shared_memory(void* ptr, size_t size) {
    if (ptr == NULL) return;
    
#ifdef _WIN32
    UnmapViewOfFile(ptr);
#else
    munmap(ptr, size);
#endif
}

void platform_unlink_shared_memory(const char* name) {
#ifdef _WIN32
    // В Windows shared memory удаляется автоматически
    (void)name;
#else
    shm_unlink(name);
#endif
}
