#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>

extern char _end;
extern char __StackTop;

static char* g_heap_end = &_end;

int _close(int file) {
    (void)file;
    return -1;
}

int _fstat(int file, struct stat* st) {
    (void)file;
    if (st == 0) {
        errno = EINVAL;
        return -1;
    }
    st->st_mode = S_IFCHR;
    return 0;
}

int _getpid(void) {
    return 1;
}

int _isatty(int file) {
    (void)file;
    return 1;
}

int _kill(int pid, int sig) {
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

int _lseek(int file, int ptr, int dir) {
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

int _read(int file, char* ptr, int len) {
    (void)file;
    (void)ptr;
    (void)len;
    return 0;
}

caddr_t _sbrk(int incr) {
    char* previous = g_heap_end;
    char* next = previous + incr;

    if (next >= &__StackTop) {
        errno = ENOMEM;
        return (caddr_t)-1;
    }

    g_heap_end = next;
    return (caddr_t)previous;
}

int _write(int file, const char* ptr, int len) {
    (void)file;
    (void)ptr;
    return len;
}

void _exit(int status) {
    (void)status;
    while (1) {
    }
}
