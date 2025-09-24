# MyShell Project Report - Person A Sections
**Author:** Mahmoud Kassem  
**Project:** Custom Remote Shell (Phase 1)  
**Course:** Operating Systems - Fall 2025  
**Due Date:** October 2nd, 2025  

---

## Title Page

**MyShell Project - Phase 1**  
**Custom Remote Shell Implementation**

**Group Members:**
- Person A: Mahmoud Kassem (NetID: [To be filled])
- Person B: [Name to be filled] (NetID: [To be filled])

**Course:** Operating Systems  
**Semester:** Fall 2025  
**Due Date:** October 2nd, 2025  
**Submission Date:** [To be filled]

---

## Architecture and Design

### High-Level System Structure

The MyShell system follows a modular architecture designed for maintainability and extensibility. The system is organized into the following main components:

#### 1. Core Components

**Main Shell Loop (`myshell.c`)**
- Implements the primary user interaction loop
- Handles prompt display and input reading
- Manages the overall shell lifecycle
- Coordinates between parsing and execution modules

**Utility Library (`shell_utils.c/h`)**
- Contains all parsing, execution, and utility functions
- Implements system call wrappers and error handling
- Provides memory management functions
- Handles file operations and redirections

#### 2. Data Structures

**Command Structure (`command_t`)**
```c
typedef struct {
    char** argv;           // Command and arguments array
    int argc;              // Number of arguments
    char* input_file;      // Input redirection filename
    char* output_file;     // Output redirection filename  
    char* error_file;      // Error redirection filename
    int has_input_redir;   // Input redirection flag
    int has_output_redir;  // Output redirection flag
    int has_error_redir;   // Error redirection flag
} command_t;
```

This structure encapsulates all information needed to execute a command, including arguments and redirection parameters.

**Error Handling System**
```c
typedef enum {
    ERROR_NONE, ERROR_MISSING_FILE, ERROR_FILE_NOT_FOUND,
    ERROR_PERMISSION_DENIED, ERROR_INVALID_COMMAND,
    ERROR_FORK_FAILED, ERROR_EXEC_FAILED, ERROR_DUP2_FAILED,
    ERROR_MALLOC_FAILED
} error_type_t;
```

### Key Design Decisions

#### 1. Modular Design Philosophy
- **Separation of Concerns:** Parsing logic is separated from execution logic
- **Single Responsibility:** Each function has a clearly defined purpose
- **Extensibility:** Design allows for easy addition of new features (pipes, etc.)

#### 2. Memory Management Strategy
- **Dynamic Allocation:** All command parsing uses dynamic memory allocation
- **Consistent Cleanup:** Every allocation has a corresponding cleanup function
- **Error Handling:** Memory allocation failures are properly handled

#### 3. Process Management
- **Fork-Exec Model:** Each command runs in a separate child process
- **Synchronous Execution:** Parent waits for child completion before next prompt
- **Proper Resource Cleanup:** File descriptors and memory are properly managed

#### 4. Error Handling Approach
- **Comprehensive Error Types:** Enumerated error types for consistent handling
- **User-Friendly Messages:** Clear error messages for different failure modes
- **Graceful Degradation:** Shell continues running after command failures

### Code Organization

#### File Structure
```
myshell/
├── myshell.c              # Main shell loop and user interface
├── shell_utils.h          # Header with function prototypes and structures
├── shell_utils.c          # Implementation of all utility functions
├── Makefile              # Build system with comprehensive targets
└── PERSON_A_IMPLEMENTATION_LOG.md  # Development tracking
```

#### Function Organization
- **Input Processing:** `parse_input()`, `parse_command_line()`, `trim_whitespace()`
- **Command Execution:** `execute_simple_command()`, `setup_redirection()`
- **Memory Management:** `free_argv()`, `free_command()`
- **Error Handling:** `handle_error()`, `validate_file_access()`

---

## Execution Instructions

### Compilation

The project uses a comprehensive Makefile for building and testing:

#### Basic Compilation
```bash
# Clean and build the project
make clean
make

# Or rebuild everything from scratch
make rebuild
```

#### Build Options
```bash
# Debug build with extra debugging symbols
make debug

# Optimized release build
make release

# View all available targets
make help
```

### Running the Program

#### Basic Usage
```bash
# Start the shell
./myshell

# The shell will display:
# MyShell v1.0 - Custom Remote Shell
# Type 'exit' to quit.
# $
```

#### Example Commands
```bash
$ ls                          # List directory contents
$ ls -l                       # List with details
$ ps aux                      # Show all processes
$ echo "Hello" > output.txt   # Output redirection
$ cat < input.txt             # Input redirection
$ ls invalid 2> error.log     # Error redirection
$ exit                        # Exit the shell
```

### Testing

The Makefile includes automated testing:

```bash
# Run basic functionality tests
make test

# Check for memory leaks (requires valgrind)
make memcheck
```

#### Manual Testing Examples
```bash
# Test basic commands
echo "ls" | ./myshell
echo "ps" | ./myshell

# Test commands with arguments
echo "ls -l" | ./myshell
echo "ps aux" | ./myshell

# Test redirections
echo "echo 'test' > output.txt" | ./myshell
echo "cat < input.txt" | ./myshell
echo "ls invalid 2> error.log" | ./myshell

# Test exit functionality
echo "exit" | ./myshell
```

### System Requirements

- **Operating System:** Linux/Unix-based system
- **Compiler:** GCC with C99 standard support
- **Dependencies:** Standard C library, POSIX system calls
- **Memory:** Minimal (< 1MB typical usage)

---

## Division of Tasks

### Person A Responsibilities (Mahmoud Kassem)

#### Core Implementation (Completed)
1. **Project Infrastructure**
   - Set up project structure and build system
   - Created comprehensive Makefile with testing targets
   - Established coding standards and documentation

2. **Basic Shell Functionality**
   - Implemented main shell loop with prompt display
   - Created input reading and parsing system
   - Handled exit command and EOF conditions

3. **Command Execution System**
   - Implemented single commands without arguments
   - Added support for commands with arguments
   - Integrated fork(), execvp(), and wait() system calls
   - Added support for executable programs

4. **Redirection System**
   - Implemented input redirection (`<`) using dup2()
   - Implemented output redirection (`>`) using dup2()
   - Implemented error redirection (`2>`) using dup2()
   - Added combined redirection support

5. **Error Handling & Memory Management**
   - Created comprehensive error handling system
   - Implemented proper memory management with cleanup functions
   - Added file access validation
   - Created user-friendly error messages

#### Documentation (Completed)
- Title Page preparation
- Architecture and Design section
- Execution Instructions
- Division of Tasks documentation
- Implementation tracking and progress logs

#### Timeline
- **Week 1 (Sept 25-29):** Project setup, basic commands, simple redirections
- **Week 2 (Sept 30 - Oct 2):** Advanced redirections, error handling, documentation

### Person B Responsibilities

#### Advanced Features (To be implemented)
1. **Pipe Implementation**
   - Single pipes (command1 | command2)
   - Multiple pipes (command1 | command2 | command3)
   - N-pipe support using pipe() system calls
   - Process synchronization for pipe chains

2. **Compound Commands**
   - Pipes with input redirection (command1 < input.txt | command2)
   - Pipes with output redirection (command1 | command2 > output.txt)
   - Pipes with error redirection (command1 | command2 2> error.log)
   - Complex combinations of all features

3. **Advanced Error Handling**
   - Pipe-specific error handling
   - Complex command validation
   - Edge case management for compound commands

#### Documentation (To be completed)
- Implementation Highlights section
- Testing section with screenshots
- Challenges and solutions
- References

### Coordination Strategy

#### Integration Points
- **Shared Data Structures:** Both persons use the `command_t` structure
- **Common Interfaces:** Person B extends Person A's parsing functions
- **Error Handling:** Consistent error handling approach across features
- **Testing:** Joint testing of integrated functionality

#### Communication Plan
- Regular code reviews and integration sessions
- Shared version control and documentation
- Joint debugging for integration issues
- Coordinated final testing and submission

---

**Note:** This document represents Person A's completed sections of the final report. Person B will complete the remaining sections (Implementation Highlights, Testing, Challenges, References) to create the complete project report.
