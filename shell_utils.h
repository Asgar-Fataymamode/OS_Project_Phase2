// #ifndef SHELL_UTILS_H
// #define SHELL_UTILS_H

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <sys/types.h>
// #include <fcntl.h>
// #include <errno.h>

// /* Constants */
// #define MAX_INPUT_SIZE 1024
// #define MAX_ARGS 64
// #define MAX_PIPES 10
// #define MAX_FILENAME 256

// /* Shell prompt and commands */
// extern const char* SHELL_PROMPT;
// extern const char* EXIT_COMMAND;

// /* Error types for better error handling */
// typedef enum {
//     ERROR_NONE,
//     ERROR_MISSING_FILE,
//     ERROR_FILE_NOT_FOUND,
//     ERROR_PERMISSION_DENIED,
//     ERROR_INVALID_COMMAND,
//     ERROR_FORK_FAILED,
//     ERROR_EXEC_FAILED,
//     ERROR_PIPE_FAILED,
//     ERROR_DUP2_FAILED,
//     ERROR_MALLOC_FAILED
// } error_type_t;

// /* Command structure for parsing */
// typedef struct {
//     char** argv;           /* Command and arguments */
//     int argc;              /* Number of arguments */
//     char* input_file;      /* Input redirection file */
//     char* output_file;     /* Output redirection file */
//     char* error_file;      /* Error redirection file */
//     int has_input_redir;   /* Flag for input redirection */
//     int has_output_redir;  /* Flag for output redirection */
//     int has_error_redir;   /* Flag for error redirection */
// } command_t;

// /* Function prototypes for basic shell operations */

// /**
//  * Parses input string into tokens (command and arguments)
//  * @param input: The input string to parse
//  * @return: Array of strings (argv), NULL-terminated
//  */
// char** parse_input(char* input);

// /**
//  * Parses a complete command line including redirections
//  * @param input: The input string to parse
//  * @return: Pointer to command_t structure, NULL on error
//  */
// command_t* parse_command_line(char* input);

// /**
//  * Executes a simple command without pipes
//  * @param cmd: Pointer to command_t structure
//  * @return: 0 on success, error code on failure
//  */
// int execute_simple_command(command_t* cmd);

// /**
//  * Sets up file redirections using dup2()
//  * @param cmd: Pointer to command_t structure
//  * @return: 0 on success, error code on failure
//  */
// int setup_redirection(command_t* cmd);

// /**
//  * Frees memory allocated for argv array
//  * @param argv: Array of strings to free
//  */
// void free_argv(char** argv);

// /**
//  * Frees memory allocated for command_t structure
//  * @param cmd: Pointer to command_t structure to free
//  */
// void free_command(command_t* cmd);

// /**
//  * Handles and displays error messages
//  * @param error: Error type from error_type_t enum
//  * @param context: Additional context string for error message
//  */
// void handle_error(error_type_t error, const char* context);

// /**
//  * Trims whitespace from beginning and end of string
//  * @param str: String to trim (modified in place)
//  * @return: Pointer to trimmed string
//  */
// char* trim_whitespace(char* str);

// /**
//  * Checks if a string is empty or contains only whitespace
//  * @param str: String to check
//  * @return: 1 if empty/whitespace only, 0 otherwise
//  */
// int is_empty_string(const char* str);

// /**
//  * Validates file access for redirection
//  * @param filename: Name of file to check
//  * @param mode: Access mode (R_OK, W_OK, etc.)
//  * @return: 1 if accessible, 0 otherwise
//  */
// int validate_file_access(const char* filename, int mode);

// #endif /* SHELL_UTILS_H */












// header files

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

// declaring these values as constants
#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64
#define MAX_PIPES 10
#define MAX_FILENAME 256

// for shell prompt and commands
extern const char* SHELL_PROMPT;
extern const char* EXIT_COMMAND;

// error types for better error handling
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

// command structure for parsing
typedef struct {
    char** argv;              // for command and arguments
    int argc;                 // number of arguments
    char* input_file;         // input redirection file
    char* output_file;       // output redirection file
    char* error_file;         // error redirection file
    int has_input_redir;      // flag for input redirection
    int has_output_redir;     // flag for output redirection
    int has_error_redir;      // flag for error redirection

    // ========= ADDED BY ASGAR =========
    int is_piped;             // flag -- to indicate whether this command is part of a pipe
    int pipe_position;        // for position in pipe chain -- using 0 = first & -1 = last
    // ========= ADDED BY ASGAR =========
} command_t;

// ========= ADDED BY ASGAR =========
// pipeline structure for handling multiple piped commands
// manages complete pipeline of commands connected by pipes
typedef struct {
    command_t** commands;     // array of commands in the pipeline
    int num_commands;         // number of commands in pipeline
    char* input_file;         // initial input redirection (for first command)
    char* output_file;        // final output redirection (for last command)
    char* error_file;         // error redirection (for last command)
} pipeline_t;
// ========= ADDED BY ASGAR =========


// function prototypes for basic shell operations

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

// ========= ADDED BY ASGAR =========
/**
 * parses input containing pipes and creates a pipeline structure
 * @param input: input string to parse
 * @return: pointer to pipeline_t structure -- NULL on error
 */
pipeline_t* parse_pipeline(char* input);

/**
 * executes a pipeline of commands connected by pipes
 * @param pipeline: pointer to pipeline_t structure
 * @return: 0 on success, error code on failure
 */
int execute_pipeline(pipeline_t* pipeline);

/**
 * frees memory allocated for pipeline_t structure
 * @param pipeline: pointer to pipeline_t structure to free
 */
void free_pipeline(pipeline_t* pipeline);

/**
 * counts the number of pipes in input string
 * @param input: input string to analyze
 * @return: number of pipe characters found
 */
int count_pipes(const char* input);

/**
 * splits input string by pipe characters
 * @param input: input string to split
 * @param num_segments: pointer to store number of segments
 * @return: array of strings (segments), NULL on error
 */
char** split_by_pipes(char* input, int* num_segments);
// ========= ADDED BY ASGAR =========

#endif /* SHELL_UTILS_H */