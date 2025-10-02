

// shell_utils.c — parsing, execution, redirection, pipelines, and errors
// Linux-safe version (no strtok_r), guards against NULL %s, fixes single-cmd redir double-free

#define _POSIX_C_SOURCE 200809L

#include "shell_utils.h"

// ---------------------------
// Small string utils
// ---------------------------

char* trim_whitespace(char* str) {
    char* end;
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') str++;
    if (*str == '\0') return str;
    end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end--;
    *(end + 1) = '\0';
    return str;
}

int is_empty_string(const char* str) {
    if (str == NULL) return 1;
    while (*str) {
        if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') return 0;
        str++;
    }
    return 1;
}

// ---------------------------
// Tokenization (argv)
// ---------------------------

char** parse_input(char* input) {
    char** argv = malloc(MAX_ARGS * sizeof(char*));
    if (!argv) { handle_error(ERROR_MALLOC_FAILED, "parse_input"); return NULL; }

    int argc = 0;
    char* token = strtok(input, " \t\n");
    while (token && argc < MAX_ARGS - 1) {
        argv[argc] = malloc(strlen(token) + 1);
        if (!argv[argc]) {
            for (int i = 0; i < argc; i++) free(argv[i]);
            free(argv);
            handle_error(ERROR_MALLOC_FAILED, "parse_input token");
            return NULL;
        }
        strcpy(argv[argc], token);
        argc++;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argv;
}

// ---------------------------
// Errors, free helpers
// ---------------------------

void handle_error(error_type_t error, const char* context) {
    const char *ctx = context ? context : "(null)";
    switch (error) {
        case ERROR_MISSING_FILE:     fprintf(stderr, "Error: Missing filename for redirection\n"); break;
        case ERROR_FILE_NOT_FOUND:   fprintf(stderr, "Error: File '%s' not found or cannot be accessed\n", ctx); break;
        case ERROR_PERMISSION_DENIED:fprintf(stderr, "Error: Permission denied for file '%s'\n", ctx); break;
        case ERROR_INVALID_COMMAND:  fprintf(stderr, "Error: Invalid command '%s'\n", ctx); break;
        case ERROR_FORK_FAILED:      fprintf(stderr, "Error: Failed to create child process for '%s'\n", ctx); break;
        case ERROR_EXEC_FAILED:      fprintf(stderr, "Error: Failed to execute command '%s'\n", ctx); break;
        case ERROR_PIPE_FAILED:      fprintf(stderr, "Error: Failed to create pipe\n"); break;
        case ERROR_DUP2_FAILED:      fprintf(stderr, "Error: Failed to set up redirection (%s)\n", ctx); break;
        case ERROR_MALLOC_FAILED:    fprintf(stderr, "Error: Memory allocation failed (%s)\n", ctx); break;
        default:                     fprintf(stderr, "Error: Unknown error occurred\n"); break;
    }
}

void free_argv(char** argv) {
    if (!argv) return;
    for (int i = 0; argv[i] != NULL; i++) free(argv[i]);
    free(argv);
}

void free_command(command_t* cmd) {
    if (!cmd) return;
    free_argv(cmd->argv);
    if (cmd->input_file)  free(cmd->input_file);
    if (cmd->output_file) free(cmd->output_file);
    if (cmd->error_file)  free(cmd->error_file);
    free(cmd);
}

void free_pipeline(pipeline_t* pipeline) {
    if (!pipeline) return;
    if (pipeline->commands) {
        for (int i = 0; i < pipeline->num_commands; i++) free_command(pipeline->commands[i]);
        free(pipeline->commands);
    }
    if (pipeline->input_file)  free(pipeline->input_file);
    if (pipeline->output_file) free(pipeline->output_file);
    if (pipeline->error_file)  free(pipeline->error_file);
    free(pipeline);
}

int validate_file_access(const char* filename, int mode) {
    return access(filename, mode) == 0;
}

// ---------------------------
// Command (with redirs) parser
// ---------------------------

command_t* parse_command_line(char* input) {
    command_t* cmd = malloc(sizeof(command_t));
    if (!cmd) { handle_error(ERROR_MALLOC_FAILED, "parse_command_line"); return NULL; }

    cmd->argv = NULL; cmd->argc = 0;
    cmd->input_file = NULL; cmd->output_file = NULL; cmd->error_file = NULL;
    cmd->has_input_redir = 0; cmd->has_output_redir = 0; cmd->has_error_redir = 0;
    cmd->is_piped = 0; cmd->pipe_position = -1;

    char* working_input = malloc(strlen(input) + 1);
    if (!working_input) { free(cmd); handle_error(ERROR_MALLOC_FAILED, "working input copy"); return NULL; }
    strcpy(working_input, input);

    char* current = working_input;

    // input '<'
    char* input_pos = strstr(current, "<");
    if (input_pos) {
        cmd->has_input_redir = 1;
        char* filename_start = input_pos + 1;
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        if (*filename_start == '\0') { handle_error(ERROR_MISSING_FILE, "input redirection"); free(working_input); free_command(cmd); return NULL; }
        char* filename_end = filename_start;
        while (*filename_end && *filename_end!=' ' && *filename_end!='\t' && *filename_end!='>' && *filename_end!='<') filename_end++;
        int filename_len = (int)(filename_end - filename_start);
        cmd->input_file = malloc(filename_len + 1);
        if (!cmd->input_file) { free(working_input); free_command(cmd); handle_error(ERROR_MALLOC_FAILED, "input filename"); return NULL; }
        strncpy(cmd->input_file, filename_start, filename_len); cmd->input_file[filename_len] = '\0';
        memmove(input_pos, filename_end, strlen(filename_end) + 1);
    }

    // stderr '2>'
    char* error_pos = strstr(current, "2>");
    if (error_pos) {
        cmd->has_error_redir = 1;
        char* filename_start = error_pos + 2;
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        if (*filename_start == '\0') { handle_error(ERROR_MISSING_FILE, "error redirection"); free(working_input); free_command(cmd); return NULL; }
        char* filename_end = filename_start;
        while (*filename_end && *filename_end!=' ' && *filename_end!='\t' && *filename_end!='>' && *filename_end!='<') filename_end++;
        int filename_len = (int)(filename_end - filename_start);
        cmd->error_file = malloc(filename_len + 1);
        if (!cmd->error_file) { free(working_input); free_command(cmd); handle_error(ERROR_MALLOC_FAILED, "error filename"); return NULL; }
        strncpy(cmd->error_file, filename_start, filename_len); cmd->error_file[filename_len] = '\0';
        memmove(error_pos, filename_end, strlen(filename_end) + 1);
    }

    // stdout '>'
    char* output_pos = strstr(current, ">");
    if (output_pos) {
        cmd->has_output_redir = 1;
        char* filename_start = output_pos + 1;
        while (*filename_start == ' ' || *filename_start == '\t') filename_start++;
        if (*filename_start == '\0') { handle_error(ERROR_MISSING_FILE, "output redirection"); free(working_input); free_command(cmd); return NULL; }
        char* filename_end = filename_start;
        while (*filename_end && *filename_end!=' ' && *filename_end!='\t' && *filename_end!='>' && *filename_end!='<') filename_end++;
        int filename_len = (int)(filename_end - filename_start);
        cmd->output_file = malloc(filename_len + 1);
        if (!cmd->output_file) { free(working_input); free_command(cmd); handle_error(ERROR_MALLOC_FAILED, "output filename"); return NULL; }
        strncpy(cmd->output_file, filename_start, filename_len); cmd->output_file[filename_len] = '\0';
        memmove(output_pos, filename_end, strlen(filename_end) + 1);
    }

    char* command_part = trim_whitespace(working_input);
    if (strlen(command_part) == 0) { handle_error(ERROR_INVALID_COMMAND, "empty command"); free(working_input); free_command(cmd); return NULL; }

    cmd->argv = parse_input(command_part);
    if (!cmd->argv) { free(working_input); free_command(cmd); return NULL; }
    cmd->argc = 0; while (cmd->argv[cmd->argc] != NULL) cmd->argc++;

    free(working_input);
    return cmd;
}

// ---------------------------
// Redirections
// ---------------------------

int setup_redirection(command_t* cmd) {
    int fd;
    if (cmd->has_input_redir) {
        if (access(cmd->input_file, R_OK) != 0) { handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file); return -1; }
        fd = open(cmd->input_file, O_RDONLY);
        if (fd == -1) { handle_error(ERROR_FILE_NOT_FOUND, cmd->input_file); return -1; }
        if (dup2(fd, STDIN_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "input redirection"); close(fd); return -1; }
        close(fd);
    }
    if (cmd->has_output_redir) {
        fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) { handle_error(ERROR_PERMISSION_DENIED, cmd->output_file); return -1; }
        if (dup2(fd, STDOUT_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "output redirection"); close(fd); return -1; }
        close(fd);
    }
    if (cmd->has_error_redir) {
        fd = open(cmd->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) { handle_error(ERROR_PERMISSION_DENIED, cmd->error_file); return -1; }
        if (dup2(fd, STDERR_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "error redirection"); close(fd); return -1; }
        close(fd);
    }
    return 0;
}

// ---------------------------
// Execute simple command
// ---------------------------

int execute_simple_command(command_t* cmd) {
    pid_t pid = fork();
    int status;
    if (pid == -1) {
        handle_error(ERROR_FORK_FAILED, cmd->argv ? cmd->argv[0] : "(null)");
        return -1;
    } else if (pid == 0) {
        if (setup_redirection(cmd) != 0) exit(EXIT_FAILURE);
        if (execvp(cmd->argv[0], cmd->argv) == -1) {
            if (errno == ENOENT) fprintf(stderr, "Error: Command not found: '%s'\n", cmd->argv[0]);
            else if (errno == EACCES) fprintf(stderr, "Error: Permission denied: '%s'\n", cmd->argv[0]);
            else handle_error(ERROR_EXEC_FAILED, cmd->argv[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        if (wait(&status) == -1) { handle_error(ERROR_INVALID_COMMAND, "wait failed"); return -1; }
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        return -1;
    }
    return 0;
}

// ---------------------------
// Pipes helpers/parsing
// ---------------------------

int count_pipes(const char* input) {
    int count = 0; const char* ptr = input; while (*ptr) { if (*ptr == '|') count++; ptr++; } return count;
}

// Manual splitter (no strtok_r). Returns heap-allocated array of heap-allocated segment strings.
char** split_by_pipes(char* input, int* num_segments) {
    char* trimmed_input = trim_whitespace(input);
    size_t len = strlen(trimmed_input);

    if (len > 0 && trimmed_input[0] == '|') { fprintf(stderr, "Error: Missing command before pipe\n"); return NULL; }
    if (len > 0 && trimmed_input[len - 1] == '|') { fprintf(stderr, "Error: Command missing after pipe\n"); return NULL; }

    const char *p = trimmed_input; int after_pipe = 0;
    while (*p) {
        if (*p == '|') { if (after_pipe) { fprintf(stderr, "Error: Empty command between pipes\n"); return NULL; } after_pipe = 1; }
        else if (*p != ' ' && *p != '\t') after_pipe = 0;
        ++p;
    }

    int capacity = count_pipes(trimmed_input) + 1;
    if (capacity < 2) { fprintf(stderr, "Error: Invalid pipeline\n"); return NULL; }

    char** segments = malloc(capacity * sizeof(char*));
    if (!segments) { handle_error(ERROR_MALLOC_FAILED, "split_by_pipes"); return NULL; }

    int seg_count = 0; const char* start = trimmed_input; const char* current = trimmed_input;
    while (*current) {
        if (*current == '|') {
            size_t seg_len = (size_t)(current - start);
            char* segment = malloc(seg_len + 1);
            if (!segment) { for (int j = 0; j < seg_count; j++) free(segments[j]); free(segments); handle_error(ERROR_MALLOC_FAILED, "segment in split_by_pipes"); return NULL; }
            strncpy(segment, start, seg_len); segment[seg_len] = '\0';

            char* trimmed_seg = trim_whitespace(segment);
            if (*trimmed_seg == '\0') { free(segment); fprintf(stderr, "Error: Empty command between pipes\n"); for (int j = 0; j < seg_count; j++) free(segments[j]); free(segments); return NULL; }

            segments[seg_count] = malloc(strlen(trimmed_seg) + 1);
            if (!segments[seg_count]) { free(segment); for (int j = 0; j < seg_count; j++) free(segments[j]); free(segments); handle_error(ERROR_MALLOC_FAILED, "segment copy"); return NULL; }
            strcpy(segments[seg_count], trimmed_seg);
            free(segment);
            seg_count++;
            start = current + 1;
        }
        current++;
    }

    size_t seg_len = (size_t)(current - start);
    char* segment = malloc(seg_len + 1);
    if (!segment) { for (int j = 0; j < seg_count; j++) free(segments[j]); free(segments); handle_error(ERROR_MALLOC_FAILED, "last segment"); return NULL; }
    strncpy(segment, start, seg_len); segment[seg_len] = '\0';

    char* trimmed_seg = trim_whitespace(segment);
    segments[seg_count] = malloc(strlen(trimmed_seg) + 1);
    if (!segments[seg_count]) { free(segment); for (int j = 0; j < seg_count; j++) free(segments[j]); free(segments); handle_error(ERROR_MALLOC_FAILED, "last segment copy"); return NULL; }
    strcpy(segments[seg_count], trimmed_seg);
    free(segment);
    seg_count++;

    *num_segments = seg_count;
    return segments;
}

// ---------------------------
// Pipeline parse & execute
// ---------------------------

pipeline_t* parse_pipeline(char* input) {
    pipeline_t* pipeline = malloc(sizeof(pipeline_t));
    if (!pipeline) { handle_error(ERROR_MALLOC_FAILED, "parse_pipeline"); return NULL; }

    pipeline->commands = NULL; pipeline->num_commands = 0;
    pipeline->input_file = NULL; pipeline->output_file = NULL; pipeline->error_file = NULL;

    if (!strchr(input, '|')) {
        pipeline->num_commands = 1;
        pipeline->commands = malloc(sizeof(command_t*));
        if (!pipeline->commands) { free(pipeline); handle_error(ERROR_MALLOC_FAILED, "pipeline commands"); return NULL; }

        pipeline->commands[0] = parse_command_line(input);
        if (!pipeline->commands[0]) { free(pipeline->commands); free(pipeline); return NULL; }
        return pipeline;
    }

    int num_segments = 0;
    char** segments = split_by_pipes(input, &num_segments);
    if (!segments) { free(pipeline); return NULL; }

    pipeline->num_commands = num_segments;
    pipeline->commands = malloc(num_segments * sizeof(command_t*));
    if (!pipeline->commands) {
        for (int i = 0; i < num_segments; i++) free(segments[i]);
        free(segments);
        free(pipeline);
        handle_error(ERROR_MALLOC_FAILED, "pipeline commands array");
        return NULL;
    }

    for (int i = 0; i < num_segments; i++) {
        pipeline->commands[i] = parse_command_line(segments[i]);
        if (!pipeline->commands[i]) {
            for (int j = 0; j < i; j++) free_command(pipeline->commands[j]);
            free(pipeline->commands);
            for (int j = 0; j < num_segments; j++) free(segments[j]);
            free(segments);
            free(pipeline);
            return NULL;
        }

        pipeline->commands[i]->is_piped = 1;
        pipeline->commands[i]->pipe_position = i;

        if (i == 0 && pipeline->commands[i]->has_input_redir) {
            pipeline->input_file = pipeline->commands[i]->input_file;
            pipeline->commands[i]->input_file = NULL;
            pipeline->commands[i]->has_input_redir = 0;
        }
        if (i == num_segments - 1) {
            if (pipeline->commands[i]->has_output_redir) {
                pipeline->output_file = pipeline->commands[i]->output_file;
                pipeline->commands[i]->output_file = NULL;
                pipeline->commands[i]->has_output_redir = 0;
            }
            if (pipeline->commands[i]->has_error_redir) {
                pipeline->error_file = pipeline->commands[i]->error_file;
                pipeline->commands[i]->error_file = NULL;
                pipeline->commands[i]->has_error_redir = 0;
            }
        }
        if (i > 0 && i < num_segments - 1) {
            if (pipeline->commands[i]->has_input_redir || pipeline->commands[i]->has_output_redir) {
                fprintf(stderr, "Error: Middle command in pipeline cannot have input/output redirections\n");
                for (int k = 0; k <= i; k++) free_command(pipeline->commands[k]);
                free(pipeline->commands);
                for (int j = 0; j < num_segments; j++) free(segments[j]);
                free(segments);
                free(pipeline);
                return NULL;
            }
        }
    }

    for (int i = 0; i < num_segments; i++) free(segments[i]);
    free(segments);
    return pipeline;
}

int execute_pipeline(pipeline_t* pipeline) {
    if (!pipeline || pipeline->num_commands == 0) { handle_error(ERROR_INVALID_COMMAND, "empty pipeline"); return -1; }

    if (pipeline->num_commands == 1) {
        if (pipeline->input_file)  { pipeline->commands[0]->input_file  = pipeline->input_file;  pipeline->commands[0]->has_input_redir  = 1; pipeline->input_file  = NULL; }
        if (pipeline->output_file) { pipeline->commands[0]->output_file = pipeline->output_file; pipeline->commands[0]->has_output_redir = 1; pipeline->output_file = NULL; }
        if (pipeline->error_file)  { pipeline->commands[0]->error_file  = pipeline->error_file;  pipeline->commands[0]->has_error_redir  = 1; pipeline->error_file  = NULL; }
        return execute_simple_command(pipeline->commands[0]);
    }

    int num_pipes = pipeline->num_commands - 1;
    int pipes[num_pipes][2];
    pid_t pids[pipeline->num_commands];

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) == -1) {
            handle_error(ERROR_PIPE_FAILED, "pipe creation");
            for (int j = 0; j < i; j++) { close(pipes[j][0]); close(pipes[j][1]); }
            return -1;
        }
    }

    for (int i = 0; i < pipeline->num_commands; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            handle_error(ERROR_FORK_FAILED, pipeline->commands[i]->argv ? pipeline->commands[i]->argv[0] : "(null)");
            for (int j = 0; j < num_pipes; j++) { close(pipes[j][0]); close(pipes[j][1]); }
            for (int j = 0; j < i; j++) waitpid(pids[j], NULL, 0);
            return -1;
        } else if (pids[i] == 0) {
            if (i == 0) {
                if (pipeline->input_file) {
                    int fd = open(pipeline->input_file, O_RDONLY);
                    if (fd == -1) { handle_error(ERROR_FILE_NOT_FOUND, pipeline->input_file); exit(EXIT_FAILURE); }
                    if (dup2(fd, STDIN_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "input redirection in pipeline"); close(fd); exit(EXIT_FAILURE); }
                    close(fd);
                }
                if (dup2(pipes[0][1], STDOUT_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "pipe output"); exit(EXIT_FAILURE); }
            } else if (i < pipeline->num_commands - 1) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "pipe input"); exit(EXIT_FAILURE); }
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "pipe output"); exit(EXIT_FAILURE); }
            } else {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "pipe input"); exit(EXIT_FAILURE); }
                if (pipeline->output_file) {
                    int fd = open(pipeline->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) { handle_error(ERROR_PERMISSION_DENIED, pipeline->output_file); exit(EXIT_FAILURE); }
                    if (dup2(fd, STDOUT_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "output redirection in pipeline"); close(fd); exit(EXIT_FAILURE); }
                    close(fd);
                }
                if (pipeline->error_file) {
                    int efd = open(pipeline->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (efd == -1) { handle_error(ERROR_PERMISSION_DENIED, pipeline->error_file); exit(EXIT_FAILURE); }
                    if (dup2(efd, STDERR_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "error redirection in pipeline"); close(efd); exit(EXIT_FAILURE); }
                    close(efd);
                }
            }

            if (pipeline->commands[i]->has_error_redir && pipeline->commands[i]->error_file) {
                int efd = open(pipeline->commands[i]->error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (efd == -1) { handle_error(ERROR_PERMISSION_DENIED, pipeline->commands[i]->error_file); exit(EXIT_FAILURE); }
                if (dup2(efd, STDERR_FILENO) == -1) { handle_error(ERROR_DUP2_FAILED, "per-command error redirection"); close(efd); exit(EXIT_FAILURE); }
                close(efd);
            }

            for (int j = 0; j < num_pipes; j++) { close(pipes[j][0]); close(pipes[j][1]); }

            if (execvp(pipeline->commands[i]->argv[0], pipeline->commands[i]->argv) == -1) {
                if (errno == ENOENT) fprintf(stderr, "Error: Command not found: '%s'\n", pipeline->commands[i]->argv[0]);
                else if (errno == EACCES) fprintf(stderr, "Error: Permission denied: '%s'\n", pipeline->commands[i]->argv[0]);
                else handle_error(ERROR_EXEC_FAILED, pipeline->commands[i]->argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < num_pipes; i++) { close(pipes[i][0]); close(pipes[i][1]); }

    int status, last_status = 0;
    for (int i = 0; i < pipeline->num_commands; i++) {
        if (waitpid(pids[i], &status, 0) == -1) { handle_error(ERROR_INVALID_COMMAND, "wait failed in pipeline"); return -1; }
        if (i == pipeline->num_commands - 1) last_status = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    }
    return last_status;
}
