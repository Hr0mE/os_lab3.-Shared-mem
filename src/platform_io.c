#include "internal/platform_io.h"

#ifdef _WIN32

#include <windows.h>
#include <direct.h>        // _mkdir

int platform_poll_stdin(int timeout_ms)
{
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE) {
        return -1;
    }

    DWORD wait_ms = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
    DWORD res = WaitForSingleObject(hStdin, wait_ms);

    if (res == WAIT_OBJECT_0) return 1;
    if (res == WAIT_TIMEOUT)  return 0;
    return -1;
}

#else  /* POSIX */

#include <unistd.h>
#include <sys/select.h>

int platform_poll_stdin(int timeout_ms)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval tv;
    struct timeval *tv_ptr = NULL;

    if (timeout_ms >= 0) {
        tv.tv_sec  = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tv_ptr = &tv;
    }

    int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, tv_ptr);

    if (ret > 0) return 1;
    if (ret == 0) return 0;
    return -1;
}

#endif

