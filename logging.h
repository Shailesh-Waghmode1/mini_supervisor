#ifndef LOGGING_H
#define LOGGING_H

#include "sysmini.h"

#define LOG_FILE "mini_supervisor.log"

// Log levels
typedef enum {
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} log_level_t;

// Logging functions
void log_init(void);
void log_message(log_level_t level, const char *message);
void log_process_start(const char *command, pid_t pid);
void log_process_exit(pid_t pid, int exit_code);
void log_process_crash(pid_t pid, int signal);
void log_process_restart(const char *command, pid_t old_pid, pid_t new_pid);

#endif