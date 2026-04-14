#include "sysmini.h"
#include "logging.h"
#include <signal.h>
#include <sys/wait.h>

// Global variables for process management
static pid_t current_child = -1;
static char current_command[MAX_CMD_LEN];
static volatile int child_exited = 0;
static int child_exit_status = 0;

// Signal handler for SIGCHLD
void sigchld_handler(int sig) {
    (void)sig; // Suppress unused parameter warning
    int status;
    pid_t pid = sys_wait4(-1, &status, WNOHANG, 0);
    
    if (pid == current_child) {
        child_exited = 1;
        child_exit_status = status;
        current_child = -1;
    }
}

// Helper function implementations
int str_len(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void str_copy(char *dest, const char *src) {
    int i = 0;
    while (src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

int str_compare(const char *s1, const char *s2) {
    int i = 0;
    while (s1[i] && s2[i]) {
        if (s1[i] != s2[i]) {
            return s1[i] - s2[i];
        }
        i++;
    }
    return s1[i] - s2[i];
}

void int_to_str(int num, char *str) {
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    int i = 0;
    int negative = 0;
    if (num < 0) {
        negative = 1;
        num = -num;
    }
    
    // Convert digits in reverse
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    if (negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse the string
    int start = 0, end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void print_str(const char *s) {
    sys_write(STDOUT_FILENO, s, str_len(s));
}

void print_error(const char *s) {
    sys_write(STDERR_FILENO, s, str_len(s));
}

// Parse command into argv array
int parse_command(char *cmd, char **argv) {
    int argc = 0;
    int i = 0;
    int start = 0;
    
    // Skip leading spaces
    while (cmd[i] == ' ') i++;
    
    while (cmd[i] && argc < 15) { // Max 15 arguments
        start = i;
        
        // Find end of current argument
        while (cmd[i] && cmd[i] != ' ') i++;
        
        // Null terminate the argument
        if (cmd[i]) {
            cmd[i] = '\0';
            i++;
        }
        
        argv[argc++] = &cmd[start];
        
        // Skip spaces
        while (cmd[i] == ' ') i++;
    }
    
    argv[argc] = 0; // NULL terminate argv
    return argc;
}

// Start a new process
pid_t start_process(const char *command) {
    pid_t pid = sys_fork();
    
    if (pid == 0) {
        // Child process
        char cmd_copy[MAX_CMD_LEN];
        char *argv[16];
        char *envp[] = {0};
        
        str_copy(cmd_copy, command);
        int argc = parse_command(cmd_copy, argv);
        
        if (argc > 0) {
            sys_execve(argv[0], argv, envp);
            // If execve returns, it failed
            print_error("Failed to execute command\n");
        }
        sys_exit(1);
    } else if (pid > 0) {
        // Parent process
        current_child = pid;
        str_copy(current_command, command);
        log_process_start(command, pid);
        return pid;
    } else {
        // Fork failed
        print_error("Failed to fork process\n");
        return -1;
    }
}

// Read a line from stdin
int read_line(char *buffer, int max_len) {
    int i = 0;
    char c;
    
    while (i < max_len - 1) {
        if (sys_read(STDIN_FILENO, &c, 1) <= 0) {
            return -1;
        }
        
        if (c == '\n') {
            break;
        }
        
        buffer[i++] = c;
    }
    
    buffer[i] = '\0';
    return i;
}

// Setup signal handler
void setup_signals(void) {
    // Simple signal setup using signal()
    signal(SIGCHLD, sigchld_handler);
}

int main(void) {
    char command[MAX_CMD_LEN];
    
    print_str("=== Mini Supervisor ===\n");
    print_str("Type commands to supervise, 'quit' to exit\n\n");
    
    // Initialize logging
    log_init();
    
    // Setup signal handling
    setup_signals();
    
    while (1) {
        // Check if child process exited
        if (child_exited) {
            child_exited = 0;
            
            if (WIFEXITED(child_exit_status)) {
                int exit_code = WEXITSTATUS(child_exit_status);
                log_process_exit(current_child, exit_code);
                
                if (exit_code != 0) {
                    print_str("Process crashed! Restarting in 2 seconds...\n");
                    // Simple sleep by reading with timeout (not perfect but works)
                    for (volatile int i = 0; i < 100000000; i++);
                    
                    pid_t new_pid = start_process(current_command);
                    if (new_pid > 0) {
                        log_process_restart(current_command, current_child, new_pid);
                        print_str("Process restarted!\n");
                    }
                }
            } else if (WIFSIGNALED(child_exit_status)) {
                int signal = WTERMSIG(child_exit_status);
                log_process_crash(current_child, signal);
                
                print_str("Process terminated by signal! Restarting...\n");
                for (volatile int i = 0; i < 50000000; i++); // Brief delay
                
                pid_t new_pid = start_process(current_command);
                if (new_pid > 0) {
                    log_process_restart(current_command, current_child, new_pid);
                    print_str("Process restarted!\n");
                }
            }
        }
        
        // Show prompt and read command
        print_str("supervisor> ");
        
        if (read_line(command, MAX_CMD_LEN) < 0) {
            break;
        }
        
        // Handle built-in commands
        if (str_compare(command, "quit") == 0) {
            if (current_child > 0) {
                print_str("Terminating supervised process...\n");
                sys_kill(current_child, SIGTERM);
                sys_wait4(current_child, 0, 0, 0);
            }
            log_message(LOG_INFO, "Mini supervisor exiting");
            break;
        }
        
        if (str_compare(command, "status") == 0) {
            if (current_child > 0) {
                print_str("Currently supervising: ");
                print_str(current_command);
                print_str(" (PID ");
                char pid_str[32];
                int_to_str(current_child, pid_str);
                print_str(pid_str);
                print_str(")\n");
            } else {
                print_str("No process currently supervised\n");
            }
            continue;
        }
        
        if (str_compare(command, "stop") == 0) {
            if (current_child > 0) {
                print_str("Stopping supervised process...\n");
                sys_kill(current_child, SIGTERM);
                current_child = -1;
                current_command[0] = '\0';
            } else {
                print_str("No process to stop\n");
            }
            continue;
        }
        
        if (str_len(command) == 0) {
            continue;
        }
        
        // If we already have a supervised process, stop it first
        if (current_child > 0) {
            print_str("Stopping current process to start new one...\n");
            sys_kill(current_child, SIGTERM);
            sys_wait4(current_child, 0, 0, 0);
        }
        
        // Start the new process
        if (start_process(command) > 0) {
            print_str("Process started and being supervised\n");
        }
    }
    
    print_str("Goodbye!\n");
    return 0;
}