#include "logging.h"
#include <sys/wait.h>

static int log_fd = -1;

void log_init(void) {
    log_fd = sys_open(LOG_FILE, O_CREAT | O_WRONLY | O_APPEND, 
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (log_fd < 0) {
        print_error("Failed to open log file\n");
        return;
    }
    log_message(LOG_INFO, "Mini supervisor started");
}

static void get_timestamp(char *buffer) {
    static int counter = 0;
    counter++;

    buffer[0] = '[';
    int_to_str(counter, buffer + 1);
    int len = str_len(buffer);
    buffer[len] = ']';
    buffer[len + 1] = ' ';
    buffer[len + 2] = '\0';
}


static const char* level_to_string(log_level_t level) {
    switch (level) {
        case LOG_INFO:  return "INFO";
        case LOG_WARN:  return "WARN";
        case LOG_ERROR: return "ERROR";
        default:        return "UNKNOWN";
    }
}

void log_message(log_level_t level, const char *message) {
    if (log_fd < 0) return;
    
    char log_entry[MAX_LOG_LEN];
    char timestamp[64];
    
    get_timestamp(timestamp);
    
    // Build log entry: [timestamp] LEVEL: message\n
    str_copy(log_entry, timestamp);
    int pos = str_len(log_entry);
    
    const char *level_str = level_to_string(level);
    str_copy(log_entry + pos, level_str);
    pos += str_len(level_str);
    
    str_copy(log_entry + pos, ": ");
    pos += 2;
    
    str_copy(log_entry + pos, message);
    pos += str_len(message);
    
    log_entry[pos] = '\n';
    log_entry[pos + 1] = '\0';
    
    sys_write(log_fd, log_entry, str_len(log_entry));
}

void log_process_start(const char *command, pid_t pid) {
    char buffer[MAX_LOG_LEN];
    str_copy(buffer, "Started process '");
    int pos = str_len(buffer);
    
    str_copy(buffer + pos, command);
    pos += str_len(command);
    
    str_copy(buffer + pos, "' with PID ");
    pos += str_len("' with PID ");
    
    int_to_str((int)pid, buffer + pos);
    
    log_message(LOG_INFO, buffer);
}

void log_process_exit(pid_t pid, int exit_code) {
    char buffer[MAX_LOG_LEN];
    str_copy(buffer, "Process ");
    int pos = str_len(buffer);
    
    int_to_str((int)pid, buffer + pos);
    pos += str_len(buffer + pos);
    
    str_copy(buffer + pos, " exited with code ");
    pos += str_len(" exited with code ");
    
    int_to_str(exit_code, buffer + pos);
    
    log_message(LOG_INFO, buffer);
}

void log_process_crash(pid_t pid, int signal) {
    char buffer[MAX_LOG_LEN];
    str_copy(buffer, "Process ");
    int pos = str_len(buffer);
    
    int_to_str((int)pid, buffer + pos);
    pos += str_len(buffer + pos);
    
    str_copy(buffer + pos, " crashed with signal ");
    pos += str_len(" crashed with signal ");
    
    int_to_str(signal, buffer + pos);
    
    log_message(LOG_ERROR, buffer);
}

void log_process_restart(const char *command, pid_t old_pid, pid_t new_pid) {
    char buffer[MAX_LOG_LEN];
    str_copy(buffer, "Restarting '");
    int pos = str_len(buffer);
    
    str_copy(buffer + pos, command);
    pos += str_len(command);
    
    str_copy(buffer + pos, "' (old PID ");
    pos += str_len("' (old PID ");
    
    int_to_str((int)old_pid, buffer + pos);
    pos += str_len(buffer + pos);
    
    str_copy(buffer + pos, ", new PID ");
    pos += str_len(", new PID ");
    
    int_to_str((int)new_pid, buffer + pos);
    pos += str_len(buffer + pos);
    
    buffer[pos] = ')';
    buffer[pos + 1] = '\0';
    
    log_message(LOG_WARN, buffer);
}