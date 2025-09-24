# MyShell - Custom Remote Shell

A custom shell implementation in C that simulates various OS services like processes, threads, and scheduling. This is Phase 1 of a multi-phase project for Operating Systems course.

## 🎯 Project Overview

MyShell is a basic shell application that can parse command-line input from users and execute commands by creating child processes. The main goal is to become familiar with Linux system calls like `fork()`, `exec()`, `wait()`, `dup2()`, and `pipe()`.

## ✨ Features Implemented (Phase 1)

### Core Shell Functionality
- ✅ Interactive shell with `$` prompt
- ✅ Command execution using `fork()`, `execvp()`, and `wait()`
- ✅ Support for commands with and without arguments
- ✅ Exit command and EOF (Ctrl+D) handling

### I/O Redirection
- ✅ **Input Redirection (`<`)** - Redirect standard input from file
- ✅ **Output Redirection (`>`)** - Redirect standard output to file  
- ✅ **Error Redirection (`2>`)** - Redirect standard error to file
- ✅ **Combined Redirections** - Support multiple redirections in one command

### Error Handling
- ✅ Comprehensive error messages for various failure scenarios
- ✅ File access validation
- ✅ Invalid command handling
- ✅ Memory management with proper cleanup

## 🚀 Getting Started

### Prerequisites
- GCC compiler with C99 standard support
- Linux/Unix-based operating system
- Make utility

### Compilation
```bash
# Clean and build
make clean
make

# Or rebuild everything
make rebuild

# Debug build
make debug

# Release build  
make release
```

### Usage
```bash
# Start the shell
./myshell

# Example commands
$ ls                              # List directory contents
$ ls -l                          # List with details
$ echo "Hello World" > output.txt # Output redirection
$ cat < input.txt                # Input redirection
$ ls invalid_dir 2> error.log    # Error redirection
$ cat < input.txt > output.txt   # Combined redirection
$ exit                           # Exit the shell
```

### Testing
```bash
# Run automated test suite
make test

# Run comprehensive feature tests
./test_person_a_features.sh

# Check for memory leaks (requires valgrind)
make memcheck
```

## 📁 Project Structure

```
myshell/
├── myshell.c                    # Main shell program
├── shell_utils.h                # Header with function prototypes
├── shell_utils.c                # Utility functions implementation
├── Makefile                     # Build system
├── test_person_a_features.sh    # Automated test suite
├── README.md                    # This file
├── .gitignore                   # Git ignore rules
└── docs/                        # Documentation
    ├── PERSON_A_IMPLEMENTATION_LOG.md
    ├── REPORT_PERSON_A_SECTIONS.md
    └── PERSON_A_COMPLETION_SUMMARY.md
```

## 🛠️ Implementation Details

### System Calls Used
- `fork()` - Create child processes for command execution
- `execvp()` - Execute commands with PATH search
- `wait()` - Parent waits for child process completion
- `dup2()` - File descriptor redirection for I/O
- `open()`, `close()` - File operations for redirections
- `access()` - File existence and permission checking

### Key Data Structures
```c
typedef struct {
    char** argv;           // Command and arguments
    int argc;              // Number of arguments
    char* input_file;      // Input redirection file
    char* output_file;     // Output redirection file
    char* error_file;      // Error redirection file
    int has_input_redir;   // Input redirection flag
    int has_output_redir;  // Output redirection flag
    int has_error_redir;   // Error redirection flag
} command_t;
```

## 📋 Upcoming Features (Phase 2+)

- 🔄 **Pipes (`|`)** - Connect commands with pipes
- 🔗 **Compound Commands** - Complex combinations of pipes and redirections
- 🧵 **Threading Support** - Multi-threaded command execution
- ⚡ **Scheduling** - Process scheduling algorithms

## 👥 Contributors

- **Mahmoud Kassem** - Person A (Core shell functionality, I/O redirection, error handling)
- **Asgar Fataymamode** - Person B (Pipe implementation, compound commands)

## 🎓 Academic Context

**Course:** Operating Systems - Fall 2025  
**Institution:** [University Name]  
**Phase:** 1 of 4  
**Due Date:** October 2nd, 2025  

## 📊 Grading Criteria Coverage

- ✅ Successful compilation with Makefile (1 pt)
- ✅ Single Commands (with/without arguments) (3 pts)
- ✅ Input, output, and error redirection (3 pts)
- 🔄 Pipes (5 pts) - *Phase 2*
- 🔄 Composed Compound Commands (10 pts) - *Phase 2*
- ✅ Error Handling (3 pts)
- ✅ Detailed & Meaningful Comments (2 pts)
- ✅ Code modularity, quality, efficiency (1.5 pts)
- ✅ Report (1.5 pts)

## 🧪 Testing Results

All implemented features pass comprehensive testing:

```
✅ Compilation successful
✅ Basic commands work
✅ Commands with arguments work  
✅ Output redirection works
✅ Input redirection works
✅ Error redirection works
✅ Combined redirections work
✅ Error handling works
✅ Memory management - no leaks
✅ Edge cases handled
```

## 📝 License

This project is developed for educational purposes as part of an Operating Systems course.

## 🤝 Contributing

This is an academic project. Contributions are limited to the assigned team members:
- Issues and suggestions are welcome
- Please follow the existing code style and commenting standards
- All contributions must maintain academic integrity

## 📞 Contact

For questions about this implementation:
- **Mahmoud Kassem** - [Contact Information]
- **Asgar Fataymamode** - [Contact Information]

---

**Status: Phase 1 Complete ✅ | Ready for Phase 2 Integration 🚀**
