#include "api/shared.h"
#include "internal/platform.h"
#include <stdio.h>

// Простая утилита для очистки shared memory

int main(void) {
    printf("Cleaning shared memory: %s\n", SHARED_MEMORY_NAME);
    platform_unlink_shared_memory(SHARED_MEMORY_NAME);
    printf("Done!\n");
    return 0;
}
