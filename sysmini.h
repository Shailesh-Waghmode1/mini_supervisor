#ifndef SYSMINI_H
#define SYSMINI_H

#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

// System call numbers for x86_64
#define SYS_READ    0
#define SYS_WRITE   1
#define SYS_OPEN    2
#define SYS_CLOSE   3
#define SYS_FORK    57
#define SYS_EXECVE  59
#define SYS_EXIT    60
#define SYS_WAIT4   61
#define SYS_KILL    62

// File flags
#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_CREAT     64
#define O_APPEND    1024

// File permissions
#define S_IRUSR     0400
#define S_IWUSR     0200
#define S_IRGRP     0040
#define S_IROTH     0004

// Wait options
#define WNOHANG     1

// Standard file descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// Buffer sizes
#define MAX_CMD_LEN   256
#define MAX_LOG_LEN   512

// Raw syscall wrapper - inline assembly for x86_64
static inline long syscall1(long num, long arg1) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a" (num), "D" (arg1)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall2(long num, long arg1, long arg2) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a" (num), "D" (arg1), "S" (arg2)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall3(long num, long arg1, long arg2, long arg3) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a" (num), "D" (arg1), "S" (arg2), "d" (arg3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

static inline long syscall4(long num, long arg1, long arg2, long arg3, long arg4) {
    long ret;
    register long r10 __asm__("r10") = arg4;
    __asm__ volatile (
        "syscall"
        : "=a" (ret)
        : "a" (num), "D" (arg1), "S" (arg2), "d" (arg3), "r" (r10)
        : "rcx", "r11", "memory"
    );
    return ret;
}

// Syscall wrappers
static inline ssize_t sys_read(int fd, void *buf, size_t count) {
    return syscall3(SYS_READ, fd, (long)buf, count);
}

static inline ssize_t sys_write(int fd, const void *buf, size_t count) {
    return syscall3(SYS_WRITE, fd, (long)buf, count);
}

static inline int sys_open(const char *pathname, int flags, mode_t mode) {
    return syscall3(SYS_OPEN, (long)pathname, flags, mode);
}

static inline int sys_close(int fd) {
    return syscall1(SYS_CLOSE, fd);
}

static inline pid_t sys_fork(void) {
    return syscall1(SYS_FORK, 0);
}

static inline int sys_execve(const char *filename, char *const argv[], char *const envp[]) {
    return syscall3(SYS_EXECVE, (long)filename, (long)argv, (long)envp);
}

static inline void sys_exit(int status) {
    syscall1(SYS_EXIT, status);
}

static inline pid_t sys_wait4(pid_t pid, int *status, int options, void *rusage) {
    return syscall4(SYS_WAIT4, pid, (long)status, options, (long)rusage);
}

static inline int sys_kill(pid_t pid, int sig) {
    return syscall2(SYS_KILL, pid, sig);
}

// Helper functions
int str_len(const char *s);
void str_copy(char *dest, const char *src);
int str_compare(const char *s1, const char *s2);
void int_to_str(int num, char *str);
void print_str(const char *s);
void print_error(const char *s);

#endif