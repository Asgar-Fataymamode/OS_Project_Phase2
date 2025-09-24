/**
 * Shell Utilities Implementation
 * 
 * This file contains all the utility functions for parsing input,
 * executing commands, handling redirections, and managing memory.
 * 
 * Author: Mahmoud Kassem (Person A)
 * Course: Operating Systems - Fall 2025
 */

#include "shell_utils.h"

/**
 * Trims whitespace from beginning and end of string
 * Modifies the string in place and returns pointer to trimmed string
 */
char* trim_whitespace(char* str) {
    char* end;
    
    /* Trim leading whitespace */
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    /* If string is all whitespace */
    if (*str == '\0') {
        return str;
    }
    
    /* Trim trailing whitespace */
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    /* Null terminate */
    *(end + 1) = '\0';
    
    return str;
}

/**
 * Checks if a string is empty or contains only whitespace
 */
int is_empty_string(const char* str) {
    if (str == NULL) return 1;
    
    while (*str) {
        if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
            return 0;
        }
        str++;
    }
    return 1;
}

/**
 * Parses input string into tokens (command and arguments)
 * Returns a NULL-terminated array of strings
 */
char** parse_input(char* input) {
    char** argv = malloc(MAX_ARGS * sizeof(char*));
    if (argv == NULL) {
        handle_error(ERROR_MALLOC_FAILED, "parse_input");
        return NULL;
    }
    
    char* token;
    int argc = 0;
    
    /* Tokenize the input string */
    token = strtok(input, " \t\n");
    while (token != NULL && argc < MAX_ARGS - 1) {
        /* Allocate memory for each token */
        argv[argc] = malloc((strlen(token) + 1) * sizeof(char));
        if (argv[argc] == NULL) {
            /* Free previously allocated memory on error */
            for (int i = 0; i < argc; i++) {
                free(argv[i]);
            }
            free(argv);
            handle_error(ERROR_MALLOC_FAILED, "parse_input token");
            return NULL;
        }
        
        strcpy(argv[argc], token);
        argc++;
        token = strtok(NULL, " \t\n");
    }
    
    /* NULL-terminate the array */
    argv[argc] = NULL;
    
    return argv;
}

/**
 * Parses a complete command line including redirections
 * Returns a pointer to command_t structure
 */
command_t* parse_command_line(char* input) {
    command_t* cmd = malloc(sizeof(command_t));
    if (cmd == NULL) {
        handle_error(ERROR_MALLOC_FAILED, "parse_command_line");
        return NULL;
    }
    
    /* Initialize command structure */
    cmd->argv = NULL;
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->error_file = NULL;
    cmd->has_input_redir = 0;
    cmd->has_output_redir = 0;
    cmd->has_error_redir = 0;
    
    /* Make a working copy of the input */
    char* working_input = malloc((strlen(input) + 1) * sizeof(char));
    if (working_input == NULL) {
        free(cmd);
        handle_error(ERROR_MALLOC_FAILED, "working input copy");
        return NULL;
    }
    strcpy(working_input, input);
    
    /* Parse redirections by finding and processing each one */
    char* current = working_input;
    
    /* Find and process input redirection (<) */
    char* input_pos = strstr(current, "<");
    if (input_pos != NULL) {
        cmd->has_input_redir = 1;
        
        /* Extract filename after < */
        char* filename_start = input_pos + 1;
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
        if (*filename_start == '\0') {
            handle_error(ERROR_MISSING_FILE, "input redirection");
            free(working_input);
            free_command(cmd);
            return NULL;
        }
        
        /* Find end of filename */
        char* filename_end = filename_start;
        while (*filename_end != '\0' && *filename_end != ' ' && 
               *filename_end != '\t' && *filename_end != '>' && 
               *filename_end != '<') {
            filename_end++;
        }
        
        /* Extract filename */
        int filename_len = filename_end - filename_start;
        cmd->input_file = malloc((filename_len + 1) * sizeof(char));
        if (cmd->input_file == NULL) {
            free(working_input);
            free_command(cmd);
            handle_error(ERROR_MALLOC_FAILED, "input filename");
            return NULL;
        }
        strncpy(cmd->input_file, filename_start, filename_len);
        cmd->input_file[filename_len] = '\0';
        
        /* Remove the redirection from the command string */
        memmove(input_pos, filename_end, strlen(filename_end) + 1);
    }
    
    /* Find and process error redirection (2>) */
    char* error_pos = strstr(current, "2>");
    if (error_pos != NULL) {
        cmd->has_error_redir = 1;
        
        /* Extract filename after 2> */
        char* filename_start = error_pos + 2;
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
        if (*filename_start == '\0') {
            handle_error(ERROR_MISSING_FILE, "error redirection");
            free(working_input);
            free_command(cmd);
            return NULL;
        }
        
        /* Find end of filename */
        char* filename_end = filename_start;
        while (*filename_end != '\0' && *filename_end != ' ' && 
               *filename_end != '\t' && *filename_end != '>' && 
               *filename_end != '<') {
            filename_end++;
        }
        
        /* Extract filename */
        int filename_len = filename_end - filename_start;
        cmd->error_file = malloc((filename_len + 1) * sizeof(char));
        if (cmd->error_file == NULL) {
            free(working_input);
            free_command(cmd);
            handle_error(ERROR_MALLOC_FAILED, "error filename");
            return NULL;
        }
        strncpy(cmd->error_file, filename_start, filename_len);
        cmd->error_file[filename_len] = '\0';
        
        /* Remove the redirection from the command string */
        memmove(error_pos, filename_end, strlen(filename_end) + 1);
    }
    
    /* Find and process output redirection (>) */
    char* output_pos = strstr(current, ">");
    if (output_pos != NULL) {
        cmd->has_output_redir = 1;
        
        /* Extract filename after > */
        char* filename_start = output_pos + 1;
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
        if (*filename_start == '\0') {
            handle_error(ERROR_MISSING_FILE, "output redirection");
            free(working_input);
            free_command(cmd);
            return NULL;
        }
        
        /* Find end of filename */
        char* filename_end = filename_start;
        while (*filename_end != '\0' && *filename_end != ' ' && 
               *filename_end != '\t' && *filename_end != '>' && 
               *filename_end != '<') {
            filename_end++;
        }
        
        /* Extract filename */
        int filename_len = filename_end - filename_start;
        cmd->output_file = malloc((filename_len + 1) * sizeof(char));
        if (cmd->output_file == NULL) {
            free(working_input);
            free_command(cmd);
            handle_error(ERROR_MALLOC_FAILED, "output filename");
            return NULL;
        }
        strncpy(cmd->output_file, filename_start, filename_len);
        cmd->output_file[filename_len] = '\0';
        
        /* Remove the redirection from the command string */
        memmove(output_pos, filename_end, strlen(filename_end) + 1);
    }
    
    /* Parse the remaining command and arguments */
    char* command_part = trim_whitespace(working_input);
    if (strlen(command_part) == 0) {
        handle_error(ERROR_INVALID_COMMAND, "empty command");
        free(working_input);
        free_command(cmd);
        return NULL;
    }
    
    /* Parse command and arguments */
    cmd->argv = parse_input(command_part);
    if (cmd->argv == NULL) {
        free(working_input);
        free_command(cmd);
        return NULL;
    }
    
    /* Count arguments */
    cmd->argc = 0;
    while (cmd->argv[cmd->argc] != NULL) {
        cmd->argc++;
    }
    
    free(working_input);
    return cmd;
}

/**
 * Sets up file redirections using dup2()
 */
int setup_redirection(command_t* cmd) {
    int fd;
    
    /* Handle input redirection */
    if (cmd->has_input_redir) {
        /* Check if file exists and is readable */
        if (access(cmd->input_file, R_OK) != 0) {
            handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file);
            return -1;
        }
        
        fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1) {
            handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file);
            return -1;
        }
        
        if (dup2(fd, STDIN_FILENO) == -1) {
            handle_error(ERROR_DUP2_FAILED, "input redirection");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    /* Handle output redirection */
    if (cmd->has_output_redir) {
        fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            handle_error(ERROR_PERMISSION_DENIED, cmd->output_file);
            return -1;
        }
        
        if (dup2(fd, STDOUT_FILENO) == -1) {
            handle_error(ERROR_DUP2_FAILED, "output redirection");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    /* Handle error redirection */
    if (cmd->has_error_redir) {
        fd = open(cmd->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            handle_error(ERROR_PERMISSION_DENIED, cmd->error_file);
            return -1;
        }
        
        if (dup2(fd, STDERR_FILENO) == -1) {
            handle_error(ERROR_DUP2_FAILED, "error redirection");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    return 0;
}

/**
 * Executes a simple command without pipes
 * Uses fork(), execvp(), and wait() system calls
 */
int execute_simple_command(command_t* cmd) {
    pid_t pid;
    int status;
    
    /* Fork a child process */
    pid = fork();
    
    if (pid == -1) {
        /* Fork failed */
        handle_error(ERROR_FORK_FAILED, cmd->argv[0]);
        return -1;
    }
    else if (pid == 0) {
        /* Child process */
        
        /* Set up redirections */
        if (setup_redirection(cmd) != 0) {
            exit(EXIT_FAILURE);
        }
        
        /* Execute the command */
        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            handle_error(ERROR_EXEC_FAILED, cmd->argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else {
        /* Parent process - wait for child to complete */
        if (wait(&status) == -1) {
            handle_error(ERROR_INVALID_COMMAND, "wait failed");
            return -1;
        }
        
        /* Check if child terminated normally */
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
    
    return 0;
}

/**
 * Validates file access for redirection
 */
int validate_file_access(const char* filename, int mode) {
    return access(filename, mode) == 0;
}

/**
 * Frees memory allocated for argv array
 */
void free_argv(char** argv) {
    if (argv == NULL) return;
    
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }
    free(argv);
}

/**
 * Frees memory allocated for command_t structure
 */
void free_command(command_t* cmd) {
    if (cmd == NULL) return;
    
    free_argv(cmd->argv);
    
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
    if (cmd->error_file) free(cmd->error_file);
    
    free(cmd);
}

/**
 * Handles and displays error messages
 */
void handle_error(error_type_t error, const char* context) {
    switch (error) {
        case ERROR_MISSING_FILE:
            fprintf(stderr, "Error: Missing filename for redirection\n");
            break;
        case ERROR_FILE_NOT_FOUND:
            fprintf(stderr, "Error: File '%s' not found or cannot be accessed\n", context);
            break;
        case ERROR_PERMISSION_DENIED:
            fprintf(stderr, "Error: Permission denied for file '%s'\n", context);
            break;
        case ERROR_INVALID_COMMAND:
            fprintf(stderr, "Error: Invalid command '%s'\n", context);
            break;
        case ERROR_FORK_FAILED:
            fprintf(stderr, "Error: Failed to create child process for '%s'\n", context);
            break;
        case ERROR_EXEC_FAILED:
            fprintf(stderr, "Error: Failed to execute command '%s'\n", context);
            break;
        case ERROR_PIPE_FAILED:
            fprintf(stderr, "Error: Failed to create pipe\n");
            break;
        case ERROR_DUP2_FAILED:
            fprintf(stderr, "Error: Failed to set up redirection (%s)\n", context);
            break;
        case ERROR_MALLOC_FAILED:
            fprintf(stderr, "Error: Memory allocation failed (%s)\n", context);
            break;
        default:
            fprintf(stderr, "Error: Unknown error occurred\n");
            break;
    }
}
