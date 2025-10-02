# MyShell — Minimal Unix-Like Shell (Operating System Project - Phase 1)

A small shell written in C. It reads user input, parses commands and redirections, and executes them via POSIX system calls. Phase 1 delivers an interactive shell with single-command execution, I/O redirection, and pipelines.

## Features

* **Core execution**

  * Executes programs with or without arguments using `fork()`, `execvp()`, and `wait()`
  * PATH search via `execvp`
  * Clean handling of `exit` and EOF (Ctrl-D)

* **Redirection**

  * Input redirection: `< file`
  * Output redirection: `> file`
  * Error redirection: `2> file`
  * Combined forms supported (e.g., `cat < in.txt > out.txt`)

* **Pipelines**

  * Command pipelines with `|` (e.g., `cmd1 | cmd2 | cmd3`)
  * Validation rules enforced:

    * The **first** command may use `<`
    * The **last** command may use `>` and/or `2>`
    * **Middle** commands cannot use `<` or `>`

* **Error handling**

  * Clear messages for missing files, permission errors, bad commands, failed `fork/exec/dup2/pipe`, and malformed pipelines
  * Defensive parsing and cleanup to avoid memory leaks

## Build and Run

**Compile**

```bash
make clean
make
```

**Run**

```bash
./myshell
```

## Usage Examples

```bash
# inside myshell
$ ls
$ ls -l
$ echo "hello" > out.txt
$ cat < out.txt
$ ls nosuchfile 2> err.log
$ printf "a\nb\nc\n" | wc -l
$ printf "mixed\nCase\n" | tr a-z A-Z > upper.txt
$ cat < input.txt | tr a-z A-Z > output.txt
$ exit
```

## Project Structure

myshell/
├── Makefile           # Build rules and convenience targets (clean, debug, release, memcheck)
├── myshell.c          # Main REPL: prompt → read → parse → execute → cleanup
├── shell_utils.h      # Shared types, constants, and function prototypes
├── shell_utils.c      # Parsing, execution, redirection, error handling, memory management
├── hello.c            # Simple C program (e.g., for testing compilation)
└── tests/              # Test data files for redirection and pipelines
    ├── testfile1.txt
    └── testfile2.txt



## Division of Tasks

### Person A — Mahmoud Kassem

* **Makefile**: Build system and convenience targets (`all`, `clean`, `rebuild`, `debug`, `release`, `memcheck`, `test`).
* **myshell.c**: Implemented the REPL (prompt display, input reading, EOF/`exit` handling), parser/executor, and per-iteration cleanup.
* **shell_utils.c**:

  * Implemented **command execution** via `execute_simple_command()` using `fork()`, `execvp()`, and `wait()`.
  * Implemented **I/O redirection** via `setup_redirection()` (`<`, `>`, `2>`) using `open()` and `dup2()`.
  * Implemented **single-command parsing**: `parse_command_line()` (detects `<`, `>`, `2>`, extracts filenames, isolates argv) and `parse_input()` (tokenization into `argv`).
  * Implemented **error handling** and utilities: `handle_error()`, `validate_file_access()`, `free_argv()`, `free_command()`.
  * Added **exec error reporting** using `errno` (user-friendly messages for `ENOENT`, `EACCES`).

### Person B — Asgar Fataymamode

* **shell_utils.h**: Defined constants and shared types (`command_t`, `pipeline_t`, `error_type_t`), declared function prototypes used across modules.

* **shell_utils.c**:

  * Implemented **pipeline support**: `count_pipes()`, **robust** `split_by_pipes()` (detects leading/trailing pipes and empty segments), and `parse_pipeline()` (builds `pipeline_t`, relocates first/last redirections to pipeline level, enforces middle-stage rules).
  * Implemented **pipeline execution**: `execute_pipeline()` (creates `pipe()` chains, sets up `dup2()` for each stage, closes FDs correctly in parent/children, aggregates exit status with `waitpid()`), plus **per-command `2>` redirection inside pipelines**.
* **myshell.c**: Integrated the pipeline path into the REPL (route all input through `parse_pipeline()` and `execute_pipeline()`), minor fixes for prompt/cleanup flow.
* **shell_utils.h**: Added `pipeline_t` structure and prototypes for pipeline parsing/execution functions.

## Contributors

* Mahmoud Kassem
* Asgar Fataymamode
