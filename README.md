# Mini Supervisor 🧩

A minimal process supervisor built using raw Linux system calls. This project demonstrates low-level process management, signal handling, and logging without relying on standard library functions.

## 🎯 What It Does

Think of this as a **tiny robot butler** that:
1. Waits for you to type a command
2. Starts that command as a separate process  
3. Watches the process: "Is it still running? Did it crash?"
4. If the process dies unexpectedly, it automatically restarts it
5. Logs everything that happens to `mini_supervisor.log`

## 🏗️ Architecture

```
mini_supervisor/
├── sysmini.h            # 🗝️ Raw syscall wrappers (tiny libc replacement)
├── main.c               # 🧠 Main supervisor loop & process management
├── logging.c            # 📜 Logging system using raw syscalls
├── logging.h            # 📜 Logging function declarations
├── crasher.c            # 💥 Test program that crashes in various ways
├── Makefile             # ⚙️ Build configuration
├── README.md            # 📄 This file
└── mini_supervisor.log  # 📝 Auto-generated log file
```

## 🔧 System Calls Used

| Operation | System Call | Purpose |
|-----------|-------------|---------|
| Read input | `read()` | Get commands from user |
| Write output | `write()` | Display messages and log entries |
| Create process | `fork()` | Create child process |
| Execute program | `execve()` | Replace child with target program |
| Wait for child | `wait4()` | Monitor child process status |
| Send signals | `kill()` | Terminate processes |
| File operations | `open()`, `close()` | Manage log file |

## 🚀 Building and Running

### Prerequisites
- Ubuntu/Debian Linux (or any Linux with GCC)
- GCC compiler
- Make utility

### Build Steps

```bash
# Clone or create the project directory
mkdir mini_supervisor
cd mini_supervisor

# Copy all the source files (sysmini.h, main.c, logging.c, etc.)

# Build everything
make all

# Optional: Run tests
make test
```

### Running the Supervisor

```bash
# Start the supervisor
./mini_supervisor
```

## 🎮 How to Use

Once started, you'll see a prompt:
```
=== Mini Supervisor ===
Type commands to supervise, 'quit' to exit

supervisor> 
```

### Available Commands

**Process Management:**
- `./crasher 0` - Test program that crashes in 3-8 seconds
- `./crasher 1` - Test program that exits with error code
- `./crasher 2` - Test program that terminates itself
- `./crasher 3` - Long-running program with occasional random crashes
- `ls -la` - Any normal Linux command
- `sleep 10` - Long-running command to test supervision

**Built-in Commands:**
- `status` - Show currently supervised process
- `stop` - Stop the current supervised process
- `quit` - Exit the supervisor

### Example Session

```bash
$ ./mini_supervisor
=== Mini Supervisor ===
Type commands to supervise, 'quit' to exit

supervisor> ./crasher 0
Process started and being supervised
Crasher started (PID: 1234, mode: 0)
Will crash in 5 seconds...
Time remaining: 5 seconds
Time remaining: 4 seconds
Time remaining: 3 seconds
Time remaining: 2 seconds
Time remaining: 1 seconds
CRASHING NOW!
Process crashed! Restarting in 2 seconds...
Process restarted!
Crasher started (PID: 1235, mode: 0)

supervisor> status
Currently supervising: ./crasher 0 (PID 1235)

supervisor> stop
Stopping supervised process...

supervisor> quit
Goodbye!
```

## 📝 Log File

The supervisor creates `mini_supervisor.log` with entries like:

```
[1] INFO: Mini supervisor started
[2] INFO: Started process './crasher 0' with PID 1234
[3] ERROR: Process 1234 crashed with signal 11
[4] WARN: Restarting './crasher 0' (old PID 1234, new PID 1235)
[5] INFO: Started process './crasher 0' with PID 1235
[6] INFO: Mini supervisor exiting
```

## 🧪 Testing Different Scenarios

### Test Crash Recovery
```bash
supervisor> ./crasher 0
# Watch it crash and restart automatically
```

### Test Error Exit Recovery  
```bash
supervisor> ./crasher 1
# Exits with code 1, gets restarted
```

### Test Normal Commands
```bash
supervisor> ls -la
# Runs normally, doesn't restart when it finishes successfully
```

### Test Long-Running Process
```bash
supervisor> ./crasher 3
# Runs indefinitely with occasional random crashes
```

## 🔍 What You Should Expect

### When Building:
```bash
$ make all
gcc -Wall -Wextra -std=c99 -O2 -g -c main.c
gcc -Wall -Wextra -std=c99 -O2 -g -c logging.c
gcc -Wall -Wextra -std=c99 -O2 -g -o mini_supervisor main.o logging.o
gcc -Wall -Wextra -std=c99 -O2 -g -o crasher crasher.c
```

### When Running:
1. **Supervisor starts** - Shows welcome message and prompt
2. **Process supervision** - Any command you type gets supervised
3. **Crash detection** - When a process crashes (non-zero exit or signal), it automatically restarts
4. **Logging** - All events are logged to `mini_supervisor.log`
5. **Clean exit** - Use `quit` to properly shut down

### Expected Behaviors:
- ✅ **Normal programs** (like `ls`) run once and don't restart
- ✅ **Crashing programs** get automatically restarted
- ✅ **Programs that exit with errors** get restarted
- ✅ **Signal-terminated programs** get restarted
- ✅ **All events are logged** with timestamps and details

## 🛠️ Technical Details

### Signal Handling
- Uses `SIGCHLD` to detect when child processes exit
- Distinguishes between normal exit, error exit, and signal termination
- Uses `WNOHANG` to avoid blocking the main loop

### Memory Management
- No dynamic memory allocation - uses fixed-size buffers
- All data structures are stack-allocated
- No memory leaks possible due to minimal memory usage

### Error Handling
- All system calls are checked for errors
- Graceful degradation when operations fail
- Detailed error logging

### Process Management
- Uses `fork()` + `execve()` pattern for process creation
- Proper process cleanup with `wait4()`
- Signal-based communication for process control

## 🚨 Limitations

- **Single process supervision** - Only supervises one process at a time
- **Simple command parsing** - Doesn't handle complex shell features
- **Basic logging** - Simple timestamp counter, not real timestamps
- **No configuration** - All behavior is hardcoded
- **Linux only** - Uses Linux-specific syscalls

## 🎯 Learning Objectives

This project teaches:
- **Low-level Linux programming** - Direct syscall usage
- **Process management** - fork, exec, wait patterns  
- **Signal handling** - Asynchronous event processing
- **System programming** - Building without standard library
- **Process supervision** - Core concept behind systemd, supervisord, etc.

## 🔧 Troubleshooting

### Build Issues:
```bash
# If make fails, try manual compilation:
gcc -Wall -Wextra -std=c99 -O2 -g -c main.c
gcc -Wall -Wextra -std=c99 -O2 -g -c logging.c  
gcc -Wall -Wextra -std=c99 -O2 -g -o mini_supervisor main.o logging.o
gcc -Wall -Wextra -std=c99 -O2 -g -o crasher crasher.c
```

### Runtime Issues:
- **Permission denied**: Make sure files are executable (`chmod +x mini_supervisor crasher`)
- **Command not found**: Use full paths (`./crasher` not `crasher`)
- **Log file issues**: Check write permissions in current directory

### Testing:
```bash
# Quick test sequence
make test
./mini_supervisor
# In supervisor: ./crasher 0
# Watch it crash and restart
# Type: quit
```

This mini supervisor demonstrates the core concepts used by production process supervisors like systemd, showing how Linux manages processes at the lowest level!