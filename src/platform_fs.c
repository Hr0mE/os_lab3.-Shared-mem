#include "internal/platform_fs.h"

#include <errno.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir_p(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #define mkdir_p(path) mkdir(path, 0755)
#endif

// Проверяем, что мы уже создали дирректорию, иначе -- создаём
int ensure_logs_dir(const char *path)
{
    if (mkdir_p(path) == 0) {
        return 0;
    }

    if (errno == EEXIST) {
        return 0;
    }

    return -1;
}
