// /**
//  * Shell Utilities Implementation
//  * 
//  * This file contains all the utility functions for parsing input,
//  * executing commands, handling redirections, and managing memory.
//  * 
//  * Author: Mahmoud Kassem (Person A)
//  * Course: Operating Systems - Fall 2025
//  */

// #include "shell_utils.h"

// /**
//  * Trims whitespace from beginning and end of string
//  * Modifies the string in place and returns pointer to trimmed string
//  */
// char* trim_whitespace(char* str) {
//     char* end;
    
//     /* Trim leading whitespace */
//     while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
//         str++;
//     }
    
//     /* If string is all whitespace */
//     if (*str == '\0') {
//         return str;
//     }
    
//     /* Trim trailing whitespace */
//     end = str + strlen(str) - 1;
//     while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
//         end--;
//     }
    
//     /* Null terminate */
//     *(end + 1) = '\0';
    
//     return str;
// }

// /**
//  * Checks if a string is empty or contains only whitespace
//  */
// int is_empty_string(const char* str) {
//     if (str == NULL) return 1;
    
//     while (*str) {
//         if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
//             return 0;
//         }
//         str++;
//     }
//     return 1;
// }

// /**
//  * Parses input string into tokens (command and arguments)
//  * Returns a NULL-terminated array of strings
//  */
// char** parse_input(char* input) {
//     char** argv = malloc(MAX_ARGS * sizeof(char*));
//     if (argv == NULL) {
//         handle_error(ERROR_MALLOC_FAILED, "parse_input");
//         return NULL;
//     }
    
//     char* token;
//     int argc = 0;
    
//     /* Tokenize the input string */
//     token = strtok(input, " \t\n");
//     while (token != NULL && argc < MAX_ARGS - 1) {
//         /* Allocate memory for each token */
//         argv[argc] = malloc((strlen(token) + 1) * sizeof(char));
//         if (argv[argc] == NULL) {
//             /* Free previously allocated memory on error */
//             for (int i = 0; i < argc; i++) {
//                 free(argv[i]);
//             }
//             free(argv);
//             handle_error(ERROR_MALLOC_FAILED, "parse_input token");
//             return NULL;
//         }
        
//         strcpy(argv[argc], token);
//         argc++;
//         token = strtok(NULL, " \t\n");
//     }
    
//     /* NULL-terminate the array */
//     argv[argc] = NULL;
    
//     return argv;
// }

// /**
//  * Parses a complete command line including redirections
//  * Returns a pointer to command_t structure
//  */
// command_t* parse_command_line(char* input) {
//     command_t* cmd = malloc(sizeof(command_t));
//     if (cmd == NULL) {
//         handle_error(ERROR_MALLOC_FAILED, "parse_command_line");
//         return NULL;
//     }
    
//     /* Initialize command structure */
//     cmd->argv = NULL;
//     cmd->argc = 0;
//     cmd->input_file = NULL;
//     cmd->output_file = NULL;
//     cmd->error_file = NULL;
//     cmd->has_input_redir = 0;
//     cmd->has_output_redir = 0;
//     cmd->has_error_redir = 0;
    
//     /* Make a working copy of the input */
//     char* working_input = malloc((strlen(input) + 1) * sizeof(char));
//     if (working_input == NULL) {
//         free(cmd);
//         handle_error(ERROR_MALLOC_FAILED, "working input copy");
//         return NULL;
//     }
//     strcpy(working_input, input);
    
//     /* Parse redirections by finding and processing each one */
//     char* current = working_input;
    
//     /* Find and process input redirection (<) */
//     char* input_pos = strstr(current, "<");
//     if (input_pos != NULL) {
//         cmd->has_input_redir = 1;
        
//         /* Extract filename after < */
//         char* filename_start = input_pos + 1;
//         while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
//         if (*filename_start == '\0') {
//             handle_error(ERROR_MISSING_FILE, "input redirection");
//             free(working_input);
//             free_command(cmd);
//             return NULL;
//         }
        
//         /* Find end of filename */
//         char* filename_end = filename_start;
//         while (*filename_end != '\0' && *filename_end != ' ' && 
//                *filename_end != '\t' && *filename_end != '>' && 
//                *filename_end != '<') {
//             filename_end++;
//         }
        
//         /* Extract filename */
//         int filename_len = filename_end - filename_start;
//         cmd->input_file = malloc((filename_len + 1) * sizeof(char));
//         if (cmd->input_file == NULL) {
//             free(working_input);
//             free_command(cmd);
//             handle_error(ERROR_MALLOC_FAILED, "input filename");
//             return NULL;
//         }
//         strncpy(cmd->input_file, filename_start, filename_len);
//         cmd->input_file[filename_len] = '\0';
        
//         /* Remove the redirection from the command string */
//         memmove(input_pos, filename_end, strlen(filename_end) + 1);
//     }
    
//     /* Find and process error redirection (2>) */
//     char* error_pos = strstr(current, "2>");
//     if (error_pos != NULL) {
//         cmd->has_error_redir = 1;
        
//         /* Extract filename after 2> */
//         char* filename_start = error_pos + 2;
//         while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
//         if (*filename_start == '\0') {
//             handle_error(ERROR_MISSING_FILE, "error redirection");
//             free(working_input);
//             free_command(cmd);
//             return NULL;
//         }
        
//         /* Find end of filename */
//         char* filename_end = filename_start;
//         while (*filename_end != '\0' && *filename_end != ' ' && 
//                *filename_end != '\t' && *filename_end != '>' && 
//                *filename_end != '<') {
//             filename_end++;
//         }
        
//         /* Extract filename */
//         int filename_len = filename_end - filename_start;
//         cmd->error_file = malloc((filename_len + 1) * sizeof(char));
//         if (cmd->error_file == NULL) {
//             free(working_input);
//             free_command(cmd);
//             handle_error(ERROR_MALLOC_FAILED, "error filename");
//             return NULL;
//         }
//         strncpy(cmd->error_file, filename_start, filename_len);
//         cmd->error_file[filename_len] = '\0';
        
//         /* Remove the redirection from the command string */
//         memmove(error_pos, filename_end, strlen(filename_end) + 1);
//     }
    
//     /* Find and process output redirection (>) */
//     char* output_pos = strstr(current, ">");
//     if (output_pos != NULL) {
//         cmd->has_output_redir = 1;
        
//         /* Extract filename after > */
//         char* filename_start = output_pos + 1;
//         while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
//         if (*filename_start == '\0') {
//             handle_error(ERROR_MISSING_FILE, "output redirection");
//             free(working_input);
//             free_command(cmd);
//             return NULL;
//         }
        
//         /* Find end of filename */
//         char* filename_end = filename_start;
//         while (*filename_end != '\0' && *filename_end != ' ' && 
//                *filename_end != '\t' && *filename_end != '>' && 
//                *filename_end != '<') {
//             filename_end++;
//         }
        
//         /* Extract filename */
//         int filename_len = filename_end - filename_start;
//         cmd->output_file = malloc((filename_len + 1) * sizeof(char));
//         if (cmd->output_file == NULL) {
//             free(working_input);
//             free_command(cmd);
//             handle_error(ERROR_MALLOC_FAILED, "output filename");
//             return NULL;
//         }
//         strncpy(cmd->output_file, filename_start, filename_len);
//         cmd->output_file[filename_len] = '\0';
        
//         /* Remove the redirection from the command string */
//         memmove(output_pos, filename_end, strlen(filename_end) + 1);
//     }
    
//     /* Parse the remaining command and arguments */
//     char* command_part = trim_whitespace(working_input);
//     if (strlen(command_part) == 0) {
//         handle_error(ERROR_INVALID_COMMAND, "empty command");
//         free(working_input);
//         free_command(cmd);
//         return NULL;
//     }
    
//     /* Parse command and arguments */
//     cmd->argv = parse_input(command_part);
//     if (cmd->argv == NULL) {
//         free(working_input);
//         free_command(cmd);
//         return NULL;
//     }
    
//     /* Count arguments */
//     cmd->argc = 0;
//     while (cmd->argv[cmd->argc] != NULL) {
//         cmd->argc++;
//     }
    
//     free(working_input);
//     return cmd;
// }

// /**
//  * Sets up file redirections using dup2()
//  */
// int setup_redirection(command_t* cmd) {
//     int fd;
    
//     /* Handle input redirection */
//     if (cmd->has_input_redir) {
//         /* Check if file exists and is readable */
//         if (access(cmd->input_file, R_OK) != 0) {
//             handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file);
//             return -1;
//         }
        
//         fd = open(cmd->input_file, O_RDONLY);
//         if (fd == -1) {
//             handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file);
//             return -1;
//         }
        
//         if (dup2(fd, STDIN_FILENO) == -1) {
//             handle_error(ERROR_DUP2_FAILED, "input redirection");
//             close(fd);
//             return -1;
//         }
//         close(fd);
//     }
    
//     /* Handle output redirection */
//     if (cmd->has_output_redir) {
//         fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
//         if (fd == -1) {
//             handle_error(ERROR_PERMISSION_DENIED, cmd->output_file);
//             return -1;
//         }
        
//         if (dup2(fd, STDOUT_FILENO) == -1) {
//             handle_error(ERROR_DUP2_FAILED, "output redirection");
//             close(fd);
//             return -1;
//         }
//         close(fd);
//     }
    
//     /* Handle error redirection */
//     if (cmd->has_error_redir) {
//         fd = open(cmd->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
//         if (fd == -1) {
//             handle_error(ERROR_PERMISSION_DENIED, cmd->error_file);
//             return -1;
//         }
        
//         if (dup2(fd, STDERR_FILENO) == -1) {
//             handle_error(ERROR_DUP2_FAILED, "error redirection");
//             close(fd);
//             return -1;
//         }
//         close(fd);
//     }
    
//     return 0;
// }

// /**
//  * Executes a simple command without pipes
//  * Uses fork(), execvp(), and wait() system calls
//  */
// int execute_simple_command(command_t* cmd) {
//     pid_t pid;
//     int status;
    
//     /* Fork a child process */
//     pid = fork();
    
//     if (pid == -1) {
//         /* Fork failed */
//         handle_error(ERROR_FORK_FAILED, cmd->argv[0]);
//         return -1;
//     }
//     else if (pid == 0) {
//         /* Child process */
        
//         /* Set up redirections */
//         if (setup_redirection(cmd) != 0) {
//             exit(EXIT_FAILURE);
//         }
        
//         /* Execute the command */
//         if (execvp(cmd->argv[0], cmd->argv) == -1) {
//             handle_error(ERROR_EXEC_FAILED, cmd->argv[0]);
//             exit(EXIT_FAILURE);
//         }
//     }
//     else {
//         /* Parent process - wait for child to complete */
//         if (wait(&status) == -1) {
//             handle_error(ERROR_INVALID_COMMAND, "wait failed");
//             return -1;
//         }
        
//         /* Check if child terminated normally */
//         if (WIFEXITED(status)) {
//             return WEXITSTATUS(status);
//         } else {
//             return -1;
//         }
//     }
    
//     return 0;
// }

// /**
//  * Validates file access for redirection
//  */
// int validate_file_access(const char* filename, int mode) {
//     return access(filename, mode) == 0;
// }

// /**
//  * Frees memory allocated for argv array
//  */
// void free_argv(char** argv) {
//     if (argv == NULL) return;
    
//     for (int i = 0; argv[i] != NULL; i++) {
//         free(argv[i]);
//     }
//     free(argv);
// }

// /**
//  * Frees memory allocated for command_t structure
//  */
// void free_command(command_t* cmd) {
//     if (cmd == NULL) return;
    
//     free_argv(cmd->argv);
    
//     if (cmd->input_file) free(cmd->input_file);
//     if (cmd->output_file) free(cmd->output_file);
//     if (cmd->error_file) free(cmd->error_file);
    
//     free(cmd);
// }

// /**
//  * Handles and displays error messages
//  */
// void handle_error(error_type_t error, const char* context) {
//     switch (error) {
//         case ERROR_MISSING_FILE:
//             fprintf(stderr, "Error: Missing filename for redirection\n");
//             break;
//         case ERROR_FILE_NOT_FOUND:
//             fprintf(stderr, "Error: File '%s' not found or cannot be accessed\n", context);
//             break;
//         case ERROR_PERMISSION_DENIED:
//             fprintf(stderr, "Error: Permission denied for file '%s'\n", context);
//             break;
//         case ERROR_INVALID_COMMAND:
//             fprintf(stderr, "Error: Invalid command '%s'\n", context);
//             break;
//         case ERROR_FORK_FAILED:
//             fprintf(stderr, "Error: Failed to create child process for '%s'\n", context);
//             break;
//         case ERROR_EXEC_FAILED:
//             fprintf(stderr, "Error: Failed to execute command '%s'\n", context);
//             break;
//         case ERROR_PIPE_FAILED:
//             fprintf(stderr, "Error: Failed to create pipe\n");
//             break;
//         case ERROR_DUP2_FAILED:
//             fprintf(stderr, "Error: Failed to set up redirection (%s)\n", context);
//             break;
//         case ERROR_MALLOC_FAILED:
//             fprintf(stderr, "Error: Memory allocation failed (%s)\n", context);
//             break;
//         default:
//             fprintf(stderr, "Error: Unknown error occurred\n");
//             break;
//     }
// }



/**
 * Shell Utilities Implementation
 * 
 * This file contains all the utility functions for parsing input, executing commands, handling redirections, and managing memory
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
    
    // ========= ADDED BY ASGAR =========
    // is_piped: 0 --> this is a standalone command, not part of pipeline
    // pipe_position: -1 --> default, indicating no specific position
    // the values will be updated later if the command is part of a pipeline
    cmd->is_piped = 0;
    cmd->pipe_position = -1;
    // ========= ADDED BY ASGAR =========
    
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

// ========= ADDED BY ASGAR =========
// counts number of pipes in input string
// need to iterates through the input string and counts how many '|' present
// number of pipes --> determines number of commands in the pipeline
int count_pipes(const char* input) {
    int count = 0;
    const char* ptr = input;
    
    // increment count each time pipe found
    while (*ptr) {
        if (*ptr == '|') {
            count++;
        }
        ptr++;
    }
    
    return count;
}

// splits input string by pipe characters into separate command strings
// tokenizes the input using '|' as delimiter
// each token becomes a separate command string that will be parsed later
char** split_by_pipes(char* input, int* num_segments) {

    // n pipes --> n+1 segments
    int pipe_count = count_pipes(input);
    *num_segments = pipe_count + 1;

    // ========= ADDED BY ASGAR =========
    // check if input ends with a pipe (missing command after pipe)
    char* trimmed_check = trim_whitespace(input);
    if (strlen(trimmed_check) > 0 && trimmed_check[strlen(trimmed_check) - 1] == '|') {
        fprintf(stderr, "Error: Command missing after pipe\n");
        return NULL;
    }
    // ========= ADDED BY ASGAR =========
    
    // Allocate array for command segments
    char** segments = malloc((*num_segments) * sizeof(char*));
    if (segments == NULL) {
        handle_error(ERROR_MALLOC_FAILED, "split_by_pipes");
        return NULL;
    }
    
    // Make a working copy of input
    char* input_copy = malloc((strlen(input) + 1) * sizeof(char));
    if (input_copy == NULL) {
        free(segments);
        handle_error(ERROR_MALLOC_FAILED, "input copy in split_by_pipes");
        return NULL;
    }
    strcpy(input_copy, input);

    // Split by pipe character
    char* token;
    char* saveptr;
    int i = 0;
    
    token = strtok_r(input_copy, "|", &saveptr);
    while (token != NULL && i < *num_segments) {
        // Trim whitespace from each segment
        token = trim_whitespace(token);

        // Check for empty command between pipes
        if (strlen(token) == 0) {
            fprintf(stderr, "Error: Empty command between pipes\n");
            // Clean up: free all previously allocated segments
            for (int j = 0; j < i; j++) {
                free(segments[j]);
            }
            free(segments);
            free(input_copy);
            return NULL;
        }
        
        // Allocate memory and copy segment string
        segments[i] = malloc((strlen(token) + 1) * sizeof(char));
        if (segments[i] == NULL) {
            // Clean up on allocation failure
            for (int j = 0; j < i; j++) {
                free(segments[j]);
            }
            free(segments);
            free(input_copy);
            handle_error(ERROR_MALLOC_FAILED, "segment in split_by_pipes");
            return NULL;
        }
        strcpy(segments[i], token);
        
        i++;
        token = strtok_r(NULL, "|", &saveptr);
    }
    
    free(input_copy);
    return segments;
}

// Parses input containing pipes and creates a pipeline structure
// main parsing function that handles both simple and complex commands
pipeline_t* parse_pipeline(char* input) {
    // Allocate and initialise pipeline structure
    pipeline_t* pipeline = malloc(sizeof(pipeline_t));
    if (pipeline == NULL) {
        handle_error(ERROR_MALLOC_FAILED, "parse_pipeline");
        return NULL;
    }
    
    // Initialize all pipeline fields to default values
    pipeline->commands = NULL;
    pipeline->num_commands = 0;
    pipeline->input_file = NULL;
    pipeline->output_file = NULL;
    pipeline->error_file = NULL;
    
    // Check for pipes in input
    if (strchr(input, '|') == NULL) {
        // No pipes found - this is a simple command
        pipeline->num_commands = 1;
        pipeline->commands = malloc(sizeof(command_t*));
        if (pipeline->commands == NULL) {
            free(pipeline);
            handle_error(ERROR_MALLOC_FAILED, "pipeline commands");
            return NULL;
        }

        // Parse the single command
        pipeline->commands[0] = parse_command_line(input);
        if (pipeline->commands[0] == NULL) {
            free(pipeline->commands);
            free(pipeline);
            return NULL;
        }
        
        return pipeline;
    }
    
    // Pipes found - split input into segments
    int num_segments;
    char** segments = split_by_pipes(input, &num_segments);
    if (segments == NULL) {
        free(pipeline);
        return NULL;
    }
    
    pipeline->num_commands = num_segments;
    
    // Allocate array for command pointers
    pipeline->commands = malloc(num_segments * sizeof(command_t*));
    if (pipeline->commands == NULL) {
        // Clean up segments on error
        for (int i = 0; i < num_segments; i++) {
            free(segments[i]);
        }
        free(segments);
        free(pipeline);
        handle_error(ERROR_MALLOC_FAILED, "pipeline commands array");
        return NULL;
    }
    
    // Parse each command segment
    for (int i = 0; i < num_segments; i++) {
        pipeline->commands[i] = parse_command_line(segments[i]);
        if (pipeline->commands[i] == NULL) {
            // Cleanup on error: free already-parsed commands and remaining segments
            for (int j = 0; j < i; j++) {
                free_command(pipeline->commands[j]);
            }
            free(pipeline->commands);
            for (int j = i; j < num_segments; j++) {
                free(segments[j]);
            }
            free(segments);
            free(pipeline);
            return NULL;
        }
        
        // Set pipe-related flags for this command
        pipeline->commands[i]->is_piped = 1;
        pipeline->commands[i]->pipe_position = i;
        
        // Handle redirection extraction for first command
        if (i == 0) {
            // Move input redirection from command to pipeline level
            if (pipeline->commands[i]->has_input_redir) {
                pipeline->input_file = pipeline->commands[i]->input_file;
                pipeline->commands[i]->input_file = NULL;
                pipeline->commands[i]->has_input_redir = 0;
            }
        }

        // Handle redirection extraction for last command
        
        if (i == num_segments - 1) {
            // Last command: can have output and error redirection
            if (pipeline->commands[i]->has_output_redir) {
                // Move output redirection from command to pipeline level
                pipeline->output_file = pipeline->commands[i]->output_file;
                pipeline->commands[i]->output_file = NULL;
                pipeline->commands[i]->has_output_redir = 0;
            }
            if (pipeline->commands[i]->has_error_redir) {
                // Move error redirection from command to pipeline level
                pipeline->error_file = pipeline->commands[i]->error_file;
                pipeline->commands[i]->error_file = NULL;
                pipeline->commands[i]->has_error_redir = 0;
            }
        }
        
        // Middle commands shouldn't have redirections
        // if (i > 0 && i < num_segments - 1) {
        //     if (pipeline->commands[i]->has_input_redir || 
        //         pipeline->commands[i]->has_output_redir || 
        //         pipeline->commands[i]->has_error_redir) {
        //         fprintf(stderr, "Error: Middle command in pipeline cannot have redirections\n");
        //         free_pipeline(pipeline);
        //         for (int j = 0; j < num_segments; j++) {
        //             free(segments[j]);
        //         }
        //         free(segments);
        //         return NULL;
        //     }
        // }
        // Handle redirections for middle and non-last commands

        // Middle commands should not have redirections, with one exception
        // error redirection (2>) can appear anywhere and applies to final command
        if (i > 0 && i < num_segments - 1) {
            // Middle commands can have error redirection (moves to pipeline level)
            if (pipeline->commands[i]->has_error_redir) {
                if (pipeline->error_file == NULL) {
                    pipeline->error_file = pipeline->commands[i]->error_file;
                    pipeline->commands[i]->error_file = NULL;
                    pipeline->commands[i]->has_error_redir = 0;
                }
            }
            
            // But middle commands cannot have input or output redirection
            if (pipeline->commands[i]->has_input_redir || 
                pipeline->commands[i]->has_output_redir) {
                fprintf(stderr, "Error: Middle command in pipeline cannot have input/output redirections\n");
                free_pipeline(pipeline);
                for (int j = 0; j < num_segments; j++) {
                    free(segments[j]);
                }
                free(segments);
                return NULL;
            }
        }

    }
    
    // Free segments (commands are now stored in pipeline)
    for (int i = 0; i < num_segments; i++) {
        free(segments[i]);
    }
    free(segments);
    
    return pipeline;
}

// free memory allocated for pipeline_t structure
// peforms complete cleanup of a pipeline structure, including: all command structures within the pipeline, all redirection filename strings, the pipeline structure itself
void free_pipeline(pipeline_t* pipeline) {
    if (pipeline == NULL) return;
    
    // free all command structures
    if (pipeline->commands) {
        for (int i = 0; i < pipeline->num_commands; i++) {
            free_command(pipeline->commands[i]);
        }
        free(pipeline->commands);
    }
    
    // free redirection filenames
    if (pipeline->input_file) free(pipeline->input_file);
    if (pipeline->output_file) free(pipeline->output_file);
    if (pipeline->error_file) free(pipeline->error_file);
    
    free(pipeline);
}

// executes a pipeline of commands connected by pipes
// needs to manage creation of pipes between commands, forking processes, setting up file descriptors, handling redirections, and waiting for all child processes to complete
int execute_pipeline(pipeline_t* pipeline) {
    if (pipeline == NULL || pipeline->num_commands == 0) {
        handle_error(ERROR_INVALID_COMMAND, "empty pipeline");
        return -1;
    }
    
    // special case: single command -- no pipes needed
    if (pipeline->num_commands == 1) {
        // restore any pipeline-level redirections to the command
        if (pipeline->input_file) {
            pipeline->commands[0]->input_file = pipeline->input_file;
            pipeline->commands[0]->has_input_redir = 1;
        }
        if (pipeline->output_file) {
            pipeline->commands[0]->output_file = pipeline->output_file;
            pipeline->commands[0]->has_output_redir = 1;
        }
        if (pipeline->error_file) {
            pipeline->commands[0]->error_file = pipeline->error_file;
            pipeline->commands[0]->has_error_redir = 1;
        }
        return execute_simple_command(pipeline->commands[0]);
    }
    
    // multiple commands with pipes -- full pipeline execution
    // for n commands, need n - 1 pipes
    int num_pipes = pipeline->num_commands - 1;

    // array of pipe file descriptors
    int pipes[num_pipes][2];  /*  */

    // array of child process IDs
    pid_t pids[pipeline->num_commands];
    
    // create all pipes before forking
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) == -1) {
            handle_error(ERROR_PIPE_FAILED, "pipe creation");
            // close any pipes already created before returning
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            return -1;
        }
    }

    // fork and execute each command
    for (int i = 0; i < pipeline->num_commands; i++) {
        pids[i] = fork();
        
        if (pids[i] == -1) {
            // fork failed
            handle_error(ERROR_FORK_FAILED, pipeline->commands[i]->argv[0]);

            // close all pipes
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // wait for any children already forked -- to prevent zombies
            for (int j = 0; j < i; j++) {
                waitpid(pids[j], NULL, 0);
            }
            
            return -1;
        }
        else if (pids[i] == 0) {
            // now for child
            
            // first command: handle input redirection from file or stdin
            if (i == 0) {
                if (pipeline->input_file) {
                    int fd = open(pipeline->input_file, O_RDONLY);
                    if (fd == -1) {
                        handle_error(ERROR_FILE_NOT_FOUND, pipeline->input_file);
                        exit(EXIT_FAILURE);
                    }
                    // redirect stdin to read from file
                    if (dup2(fd, STDIN_FILENO) == -1) {
                        handle_error(ERROR_DUP2_FAILED, "input redirection in pipeline");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                    // close original fd, stdin now points to file
                    close(fd);
                }

                // write to next pipe
                if (dup2(pipes[0][1], STDOUT_FILENO) == -1) {
                    handle_error(ERROR_DUP2_FAILED, "pipe output");
                    exit(EXIT_FAILURE);
                }
            }
            // Handle middle commands -- handles: cmd2 in "cmd1 | cmd2 | cmd3"
            // input is output from previous pipe, output is input to next pipe
            else if (i < pipeline->num_commands - 1) {
                // read from previous pipe
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    handle_error(ERROR_DUP2_FAILED, "pipe input");
                    exit(EXIT_FAILURE);
                }
                
                // redict output to next pipe
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    handle_error(ERROR_DUP2_FAILED, "pipe output");
                    exit(EXIT_FAILURE);
                }
            }
            //last command
            // handles: cmd3 in "cmd1 | cmd2 | cmd3"
            else {
                // Read from previous pipe
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    handle_error(ERROR_DUP2_FAILED, "pipe input");
                    exit(EXIT_FAILURE);
                }
                
                // Write to output file or stdout
                if (pipeline->output_file) {
                    int fd = open(pipeline->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        handle_error(ERROR_PERMISSION_DENIED, pipeline->output_file);
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        handle_error(ERROR_DUP2_FAILED, "output redirection in pipeline");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                    close(fd);
                }

                // Handle error redirection for last command
                if (pipeline->error_file) {
                    int fd = open(pipeline->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        handle_error(ERROR_PERMISSION_DENIED, pipeline->error_file);
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(fd, STDERR_FILENO) == -1) {
                        handle_error(ERROR_DUP2_FAILED, "error redirection in pipeline");
                        close(fd);
                        exit(EXIT_FAILURE);
                    }
                    close(fd);
                }
            }
            
            // close all pipe file descriptors in child
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // execute the command
            if (execvp(pipeline->commands[i]->argv[0], pipeline->commands[i]->argv) == -1) {
                handle_error(ERROR_EXEC_FAILED, pipeline->commands[i]->argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    // parent process continues here after fork()
    
    // parent process: close all pipes
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // parent must wait for all child processes to complete
    int status;
    int last_status = 0;
    for (int i = 0; i < pipeline->num_commands; i++) {
        if (waitpid(pids[i], &status, 0) == -1) {
            handle_error(ERROR_INVALID_COMMAND, "wait failed in pipeline");
            return -1;
        }
        
        // store the exit status of the last command
        if (i == pipeline->num_commands - 1) {
            if (WIFEXITED(status)) {
                last_status = WEXITSTATUS(status);
            } else {
                last_status = -1;
            }
        }
    }
    
    return last_status;
}

// ========= ADDED BY ASGAR =========