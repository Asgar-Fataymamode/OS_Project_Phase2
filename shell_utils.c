// this file --> contains all the utility functions for parsing input, executing commands, handling redirections, and managing memory
#define _POSIX_C_SOURCE 200809L

#include "shell_utils.h"

// trims whitespace from beginning and end of string -- modifies the string in place and returns pointer to trimmed string
char* trim_whitespace(char* str) {
    char* end;
    
    // trim leading whitespace
    // move pointer forward past any leading whitespace characters
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    
    // check if string is all whitespace -- just return the empty string
    if (*str == '\0') {
        return str;
    }
    
    // trim trailing whitespace
    // find the last character in the string
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    
    // null terminate
    // place null terminator right after the last non-whitespace character
    *(end + 1) = '\0';
    
    return str;
}

// check if a string is empty or contains only whitespace
int is_empty_string(const char* str) {

    // null pointer is considered empty
    if (str == NULL) return 1;
    
    // iterate through each character in the string
    while (*str) {
        // if we find any character that's not whitespace, string is not empty
        if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
            return 0;
        }

        // move to next character
        str++;
    }
    return 1;
}

// parses input string into tokens (command and arguments)-- returns a NULL-terminated array of strings
char** parse_input(char* input) {

    // allocate array to hold pointers to argument strings
    char** argv = malloc(MAX_ARGS * sizeof(char*));

    if (argv == NULL) {
        handle_error(ERROR_MALLOC_FAILED, "parse_input");
        return NULL;
    }
    
    char* token;

    // tracks how many arguments were parsed
    int argc = 0;
    
    // strtok splits string by delimiters -- space, tab, newline
    token = strtok(input, " \t\n");

    // keep getting tokens until strtok returns NULL -- no more tokens
     // also stop if we've reached maximum number of arguments - 1
    while (token != NULL && argc < MAX_ARGS - 1) {

        // allocate memory to store this token
        // strlen + 1 to include space for null terminator
        argv[argc] = malloc((strlen(token) + 1) * sizeof(char));

        if (argv[argc] == NULL) {
            // if allocation fails --> need to clean up everything we've allocated so far
            for (int i = 0; i < argc; i++) {
                 // free each string already allocated
                free(argv[i]);
            }

            // free the argv array itself
            free(argv);
            handle_error(ERROR_MALLOC_FAILED, "parse_input token");
            return NULL;
        }
        
        // copy the token string into our allocated memory
        strcpy(argv[argc], token);
        argc++;

        // get next token
        token = strtok(NULL, " \t\n");
    }
    
    // null-terminate the array so we know where it ends
    argv[argc] = NULL;
    
    return argv;
}

// parses complete command line including redirections -- returns a pointer to command_t structure
command_t* parse_command_line(char* input) {

    // allocate memory for the command structure
    command_t* cmd = malloc(sizeof(command_t));
    if (cmd == NULL) {
        handle_error(ERROR_MALLOC_FAILED, "parse_command_line");
        return NULL;
    }
    
    // initialize all fields to safe default values
    cmd->argv = NULL;
    cmd->argc = 0;
    cmd->input_file = NULL;
    cmd->output_file = NULL;
    cmd->error_file = NULL;
    cmd->has_input_redir = 0;
    cmd->has_output_redir = 0;
    cmd->has_error_redir = 0;
    
    // initialize pipe-related fields
    // is_piped: 0 --> this is a standalone command, not part of pipeline
    // pipe_position: -1 --> default, indicating no specific position
    // the values will be updated later if the command is part of a pipeline
    cmd->is_piped = 0;
    cmd->pipe_position = -1;
    
    // make a working copy of the input string -- must not modify the original input
    char* working_input = malloc((strlen(input) + 1) * sizeof(char));
    if (working_input == NULL) {
        free(cmd);
        handle_error(ERROR_MALLOC_FAILED, "working input copy");
        return NULL;
    }
    strcpy(working_input, input);
    
    // pointer to current position in string as we parse
    char* current = working_input;
    
    // search for '<' char in the string
    char* input_pos = strstr(current, "<");
    if (input_pos != NULL) {
        cmd->has_input_redir = 1;
        
        // move past the '<' char to find the filename
        char* filename_start = input_pos + 1;

        // skip any whitespace after the '<'
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
        // check whether there is filename -- if we hit end of string ==> error
        if (*filename_start == '\0') {
            handle_error(ERROR_MISSING_FILE, "input redirection");
            free(working_input);
            free_command(cmd);
            return NULL;
        }
        
        // find where the filename ends
        // the conditions where we stop at: end of string, whitespace, or another redirection operator
        char* filename_end = filename_start;
        while (*filename_end != '\0' && *filename_end != ' ' && 
               *filename_end != '\t' && *filename_end != '>' && 
               *filename_end != '<') {
            filename_end++;
        }
        
        // calculate length of filename
        int filename_len = filename_end - filename_start;

        // need to allocate memory to store the filename
        cmd->input_file = malloc((filename_len + 1) * sizeof(char));
        if (cmd->input_file == NULL) {
            free(working_input);
            free_command(cmd);
            handle_error(ERROR_MALLOC_FAILED, "input filename");
            return NULL;
        }

        // copy exactly filename_len characters into the buffer
        strncpy(cmd->input_file, filename_start, filename_len);

        // manually add null terminator
        cmd->input_file[filename_len] = '\0';
        
        // remove the entire '< filename' part from the command string
        memmove(input_pos, filename_end, strlen(filename_end) + 1);
    }
    
    // check for 2> before checking for > to avoid confusion
    // '2>' must be found as a unit -- not as separate '2' and '>'
    char* error_pos = strstr(current, "2>");
    if (error_pos != NULL) {
        cmd->has_error_redir = 1;
        
        // extract filename after 2
        char* filename_start = error_pos + 2;

        // skip whitespace after '2>'
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
        // check for missing filename
        if (*filename_start == '\0') {
            handle_error(ERROR_MISSING_FILE, "error redirection");
            free(working_input);
            free_command(cmd);
            return NULL;
        }
        
        // find end of filename -- same logic as input redirection
        char* filename_end = filename_start;
        while (*filename_end != '\0' && *filename_end != ' ' && 
               *filename_end != '\t' && *filename_end != '>' && 
               *filename_end != '<') {
            filename_end++;
        }
        
        // extract filename
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
        
        // remove the redirection from the command string
        memmove(error_pos, filename_end, strlen(filename_end) + 1);
    }
    
    // now safe to look for '>' because we already handled '2>'
    char* output_pos = strstr(current, ">");
    if (output_pos != NULL) {
        cmd->has_output_redir = 1;
        
        // move past the '>' character
        char* filename_start = output_pos + 1;

        // skip whitespace
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        
        // check for missing filename
        if (*filename_start == '\0') {
            handle_error(ERROR_MISSING_FILE, "output redirection");
            free(working_input);
            free_command(cmd);
            return NULL;
        }
        
        // find end of filename
        char* filename_end = filename_start;
        while (*filename_end != '\0' && *filename_end != ' ' && 
               *filename_end != '\t' && *filename_end != '>' && 
               *filename_end != '<') {
            filename_end++;
        }
        
        // extract and store filename
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
        
        // remove the redirection from the command string
        memmove(output_pos, filename_end, strlen(filename_end) + 1);
    }
    
    // parse the remaining command and arguments
    // after removing all redirections, just the command and args left

    // trim trailing whitespace
    char* command_part = trim_whitespace(working_input);

    // if nothing left after removing redirections --> error
    if (strlen(command_part) == 0) {
        handle_error(ERROR_INVALID_COMMAND, "empty command");
        free(working_input);
        free_command(cmd);
        return NULL;
    }
    
    // parse the command and arguments into argv array
    cmd->argv = parse_input(command_part);
    if (cmd->argv == NULL) {
        free(working_input);
        free_command(cmd);
        return NULL;
    }
    
    // walk through argv until hit the NULL terminator
    cmd->argc = 0;
    while (cmd->argv[cmd->argc] != NULL) {
        cmd->argc++;
    }
    
    // free working copy -- return the fully parsed command structure
    free(working_input);
    return cmd;
}

//Sets up file redirections using dup2()
int setup_redirection(command_t* cmd) {
    int fd;
    
     // handle input redirection if present
    if (cmd->has_input_redir) {
        // check if file exists and is readable
        if (access(cmd->input_file, R_OK) != 0) {
            handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file);
            return -1;
        }
        
        // open the file for reading
        fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1) {
            handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file);
            return -1;
        }
        
        // redirect stdin to read from this file
        if (dup2(fd, STDIN_FILENO) == -1) {
            handle_error(ERROR_DUP2_FAILED, "input redirection");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    // handle output redirection if present
    if (cmd->has_output_redir) {

        // open file for writing -- need to create if does not exist + truncate if it does
        fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            handle_error(ERROR_PERMISSION_DENIED, cmd->output_file);
            return -1;
        }
        
        // redirect stdout to write to this file
        if (dup2(fd, STDOUT_FILENO) == -1) {
            handle_error(ERROR_DUP2_FAILED, "output redirection");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    // handle error redirection if present
    if (cmd->has_error_redir) {
        // open file for writing
        fd = open(cmd->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            handle_error(ERROR_PERMISSION_DENIED, cmd->error_file);
            return -1;
        }
        
        // redirect stderr to write to this file
        if (dup2(fd, STDERR_FILENO) == -1) {
            handle_error(ERROR_DUP2_FAILED, "error redirection");
            close(fd);
            return -1;
        }
        close(fd);
    }
    
    return 0;
}

// executes simple command without pipes -- using fork(), execvp(), and wait() system calls
int execute_simple_command(command_t* cmd) {
    pid_t pid;
    int status;
    
    // create a child process
    pid = fork();
    
    if (pid == -1) {
        // fork has failed
        handle_error(ERROR_FORK_FAILED, cmd->argv[0]);
        return -1;
    }
    else if (pid == 0) {
        // this si child process
        
        // set up any file redirections
        if (setup_redirection(cmd) != 0) {
            exit(EXIT_FAILURE);
        }
        
        // replace child process with the command to run
        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            // handle_error(ERROR_EXEC_FAILED, cmd->argv[0]);
            // exit(EXIT_FAILURE);
            // Check errno to provide more specific error messages
            if (errno == ENOENT) {
                fprintf(stderr, "Error: Command not found: '%s'\n", cmd->argv[0]);
            } 
            else if (errno == EACCES) {
                fprintf(stderr, "Error: Permission denied: '%s'\n", cmd->argv[0]);
            } 
            else {
                handle_error(ERROR_EXEC_FAILED, cmd->argv[0]);
            }
            exit(EXIT_FAILURE);
        }
    }
    else {
        // parent process --  wait for child to finish executing
        if (wait(&status) == -1) {
            handle_error(ERROR_INVALID_COMMAND, "wait failed");
            return -1;
        }
        
        // check if child exited normally and get its exit status
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        } else {
            return -1;
        }
    }
    
    return 0;
}

// validate file access for redirection
int validate_file_access(const char* filename, int mode) {
    return access(filename, mode) == 0;
}

// free memory allocated for argv array
void free_argv(char** argv) {
    if (argv == NULL) return;
    
    // free each individual string
    for (int i = 0; argv[i] != NULL; i++) {
        free(argv[i]);
    }

    // need to free the array itself
    free(argv);
}

// free memory allocated for command_t structure
void free_command(command_t* cmd) {
    if (cmd == NULL) return;
    
    // free the argument array
    free_argv(cmd->argv);
    
    // free any redirection filenames
    if (cmd->input_file) free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
    if (cmd->error_file) free(cmd->error_file);
    
    // free the command structure
    free(cmd);
}

// handles and displays errors
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


char** split_by_pipes(char* input, int* num_segments) {
    // trim once to inspect structure
    char* trimmed_input = trim_whitespace(input);
    size_t len = strlen(trimmed_input);

    // pipe at beginning
    if (len > 0 && trimmed_input[0] == '|') {
        fprintf(stderr, "Error: Missing command before pipe\n");
        return NULL;
    }
    // pipe at end
    if (len > 0 && trimmed_input[len - 1] == '|') {
        fprintf(stderr, "Error: Command missing after pipe\n");
        return NULL;
    }

    // detect empty command between pipes
    {
        const char *p = trimmed_input;
        int after_pipe = 0;
        while (*p) {
            if (*p == '|') {
                if (after_pipe) {
                    fprintf(stderr, "Error: Empty command between pipes\n");
                    return NULL;
                }
                after_pipe = 1;
            } else if (*p != ' ' && *p != '\t') {
                after_pipe = 0;
            }
            ++p;
        }
    }

    // upper bound capacity
    int capacity = count_pipes(trimmed_input) + 1;
    if (capacity < 2) {
        fprintf(stderr, "Error: Invalid pipeline\n");
        return NULL;
    }

    char** segments = malloc(capacity * sizeof(char*));
    if (!segments) {
        handle_error(ERROR_MALLOC_FAILED, "split_by_pipes");
        return NULL;
    }

    // work on a modifiable copy for strtok_r
    char* input_copy = malloc((strlen(trimmed_input) + 1) * sizeof(char));
    if (!input_copy) {
        free(segments);
        handle_error(ERROR_MALLOC_FAILED, "input copy in split_by_pipes");
        return NULL;
    }
    strcpy(input_copy, trimmed_input);

    // tokenize by '|'
    int i = 0;
    char *saveptr = NULL;
    char *token = strtok_r(input_copy, "|", &saveptr);

    while (token != NULL) {
        char* t = trim_whitespace(token);

        if (*t == '\0') {
            fprintf(stderr, "Error: Empty command between pipes\n");
            for (int j = 0; j < i; j++) free(segments[j]);
            free(segments);
            free(input_copy);
            return NULL;
        }

        segments[i] = malloc((strlen(t) + 1) * sizeof(char));
        if (!segments[i]) {
            for (int j = 0; j < i; j++) free(segments[j]);
            free(segments);
            free(input_copy);
            handle_error(ERROR_MALLOC_FAILED, "segment in split_by_pipes");
            return NULL;
        }
        strcpy(segments[i], t);
        i++;

        token = strtok_r(NULL, "|", &saveptr);
    }

    *num_segments = i;

    free(input_copy);
    return segments;
}









// parses input containing pipes and creates a pipeline structure
// main parsing function --> handles both simple and complex commands
pipeline_t* parse_pipeline(char* input) {
    // allocate and initialise pipeline structure
    pipeline_t* pipeline = malloc(sizeof(pipeline_t));
    if (pipeline == NULL) {
        handle_error(ERROR_MALLOC_FAILED, "parse_pipeline");
        return NULL;
    }
    
    // need to initialise all pipeline fields to default values
    pipeline->commands = NULL;
    pipeline->num_commands = 0;
    pipeline->input_file = NULL;
    pipeline->output_file = NULL;
    pipeline->error_file = NULL;
    
    // check for pipes in input
    if (strchr(input, '|') == NULL) {
        // if no pipes found - this is a simple command
        pipeline->num_commands = 1;
        pipeline->commands = malloc(sizeof(command_t*));
        if (pipeline->commands == NULL) {
            free(pipeline);
            handle_error(ERROR_MALLOC_FAILED, "pipeline commands");
            return NULL;
        }

        // else parse the single command
        pipeline->commands[0] = parse_command_line(input);
        if (pipeline->commands[0] == NULL) {
            free(pipeline->commands);
            free(pipeline);
            return NULL;
        }
        
        return pipeline;
    }
    
    // pipes found - need to split input into segments
    int num_segments;
    char** segments = split_by_pipes(input, &num_segments);
    if (segments == NULL) {
        free(pipeline);
        return NULL;
    }
    
    pipeline->num_commands = num_segments;
    
    // allocate array to hold pointers to each command
    pipeline->commands = malloc(num_segments * sizeof(command_t*));
    if (pipeline->commands == NULL) {
        // clean up segments on error
        for (int i = 0; i < num_segments; i++) {
            free(segments[i]);
        }
        free(segments);
        free(pipeline);
        handle_error(ERROR_MALLOC_FAILED, "pipeline commands array");
        return NULL;
    }
    
    // parse each command segment
    for (int i = 0; i < num_segments; i++) {
        pipeline->commands[i] = parse_command_line(segments[i]);
        if (pipeline->commands[i] == NULL) {
            // clean up on error -- free already parsed commands and remaining segments
            for (int j = 0; j < i; j++) {
                free_command(pipeline->commands[j]);
            }
            free(pipeline->commands);

            // free remaining unparsed segments
            for (int j = i; j < num_segments; j++) {
                free(segments[j]);
            }
            free(segments);
            free(pipeline);
            return NULL;
        }
        
        // mark this command as part of a pipeline
        pipeline->commands[i]->is_piped = 1;
        pipeline->commands[i]->pipe_position = i;
        
        // first command: handle input redirection
        if (i == 0) {
            // move input redirection from command level --> pipeline level
            if (pipeline->commands[i]->has_input_redir) {
                pipeline->input_file = pipeline->commands[i]->input_file;
                pipeline->commands[i]->input_file = NULL;
                pipeline->commands[i]->has_input_redir = 0;
            }
        }

        // handle redirection extraction for last command
        if (i == num_segments - 1) {
            if (pipeline->commands[i]->has_output_redir) {
                // move output redirection from command to pipeline level
                pipeline->output_file = pipeline->commands[i]->output_file;
                pipeline->commands[i]->output_file = NULL;
                pipeline->commands[i]->has_output_redir = 0;
            }
            if (pipeline->commands[i]->has_error_redir) {
                // move error redirection as well to pipeline level
                pipeline->error_file = pipeline->commands[i]->error_file;
                pipeline->commands[i]->error_file = NULL;
                pipeline->commands[i]->has_error_redir = 0;
            }
        }
        
        // middle commands -- validate redirection rules
        if (i > 0 && i < num_segments - 1) {
                if (pipeline->commands[i]->has_input_redir || pipeline->commands[i]->has_output_redir) {
                fprintf(stderr, "Error: Middle command in pipeline cannot have input/output redirections\n");

                // free only initialized commands 0..i
                for (int k = 0; k <= i; k++) {
                    free_command(pipeline->commands[k]);
                }
                free(pipeline->commands);
                free(pipeline);

                for (int j = 0; j < num_segments; j++) free(segments[j]);
                free(segments);
                return NULL;
            }
        }


    }
    
    // free the segment strings
    for (int i = 0; i < num_segments; i++) {
        free(segments[i]);
    }
    free(segments);
    
    return pipeline;
}

// free memory allocated for pipeline_t structure
// peforms complete cleanup of a pipeline including: all command structures within the pipeline, all redirection filename strings + pipeline structure itself
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
    
    // free the pipeline structure itself
    free(pipeline);
}

// executes a pipeline of commands connected by pipes
// needs to manage creation of pipes between commands, forking processes, setting up file descriptors, handling redirections, waiting for all child processes to complete
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
    // for n commands --> need n - 1 pipes
    int num_pipes = pipeline->num_commands - 1;

    // array of pipe file descriptors -- read one end, write the other end
    int pipes[num_pipes][2];

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
            
            // need to prevent zombies -- therefore wait for any children already forked
            for (int j = 0; j < i; j++) {
                waitpid(pids[j], NULL, 0);
            }
            
            return -1;
        }
        else if (pids[i] == 0) {
            // now for child
            
            // first command: handle input redirection from file
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
                    // close original fd
                    close(fd);
                }

                // write to next pipe
                if (dup2(pipes[0][1], STDOUT_FILENO) == -1) {
                    handle_error(ERROR_DUP2_FAILED, "pipe output");
                    exit(EXIT_FAILURE);
                }
            }
            // handle middle commands -- handles: cmd2 in 'cmd1 | cmd2 | cmd3'
            // --> input is output from previous pipe, output is input to next pipe
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
            // last command
            // handles: cmd3 in 'cmd1 | cmd2 | cmd3'
            else {
                // read from previous pipe
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    handle_error(ERROR_DUP2_FAILED, "pipe input");
                    exit(EXIT_FAILURE);
                }
                
                // write to output file or stdout
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

                // handle error redirection for last command
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

            // handle per command stderr redirection
            if (pipeline->commands[i]->has_error_redir && pipeline->commands[i]->error_file) {
                int efd = open(pipeline->commands[i]->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if (efd == -1) {
                    handle_error(ERROR_PERMISSION_DENIED, pipeline->commands[i]->error_file);
                    exit(EXIT_FAILURE);
                }
                if (dup2(efd, STDERR_FILENO) == -1) {
                    handle_error(ERROR_DUP2_FAILED, "per-command error redirection");
                    close(efd);
                    exit(EXIT_FAILURE);
                }
                close(efd);
            }
            
            // close all pipe file descriptors in child
            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // need to replace child --> command
            if (execvp(pipeline->commands[i]->argv[0], pipeline->commands[i]->argv) == -1) {
                handle_error(ERROR_EXEC_FAILED, pipeline->commands[i]->argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    // parent process continues here after fork()
    
    // close all pipes in parent process
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
