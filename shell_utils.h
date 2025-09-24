#ifndef SHELL_UTILS_H
#define SHELL_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

/* Constants */
#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_PIPES 10
#define MAX_FILENAME 256

/* Shell prompt and commands */
extern const char* SHELL_PROMPT;
extern const char* EXIT_COMMAND;

/* Error types for better error handling */
typedef enum {
    ERROR_NONE,
    ERROR_MISSING_FILE,
    ERROR_FILE_NOT_FOUND,
    ERROR_PERMISSION_DENIED,
    ERROR_INVALID_COMMAND,
    ERROR_FORK_FAILED,
    ERROR_EXEC_FAILED,
    ERROR_PIPE_FAILED,
    ERROR_DUP2_FAILED,
    ERROR_MALLOC_FAILED
} error_type_t;

/* Command structure for parsing */
typedef struct {
    char** argv;           /* Command and arguments */
    int argc;              /* Number of arguments */
    char* input_file;      /* Input redirection file */
    char* output_file;     /* Output redirection file */
    char* error_file;      /* Error redirection file */
    int has_input_redir;   /* Flag for input redirection */
    int has_output_redir;  /* Flag for output redirection */
    int has_error_redir;   /* Flag for error redirection */
} command_t;

/* Function prototypes for basic shell operations */

/**
 * Parses input string into tokens (command and arguments)
 * @param input: The input string to parse
 * @return: Array of strings (argv), NULL-terminated
 */
char** parse_input(char* input);

/**
 * Parses a complete command line including redirections
 * @param input: The input string to parse
 * @return: Pointer to command_t structure, NULL on error
 */
command_t* parse_command_line(char* input);

/**
 * Executes a simple command without pipes
 * @param cmd: Pointer to command_t structure
 * @return: 0 on success, error code on failure
 */
int execute_simple_command(command_t* cmd);

/**
 * Sets up file redirections using dup2()
 * @param cmd: Pointer to command_t structure
 * @return: 0 on success, error code on failure
 */
int setup_redirection(command_t* cmd);

/**
 * Frees memory allocated for argv array
 * @param argv: Array of strings to free
 */
void free_argv(char** argv);

/**
 * Frees memory allocated for command_t structure
 * @param cmd: Pointer to command_t structure to free
 */
void free_command(command_t* cmd);

/**
 * Handles and displays error messages
 * @param error: Error type from error_type_t enum
 * @param context: Additional context string for error message
 */
void handle_error(error_type_t error, const char* context);

/**
 * Trims whitespace from beginning and end of string
 * @param str: String to trim (modified in place)
 * @return: Pointer to trimmed string
 */
char* trim_whitespace(char* str);

/**
 * Checks if a string is empty or contains only whitespace
 * @param str: String to check
 * @return: 1 if empty/whitespace only, 0 otherwise
 */
int is_empty_string(const char* str);

/**
 * Validates file access for redirection
 * @param filename: Name of file to check
 * @param mode: Access mode (R_OK, W_OK, etc.)
 * @return: 1 if accessible, 0 otherwise
 */
int validate_file_access(const char* filename, int mode);

#endif /* SHELL_UTILS_H */
