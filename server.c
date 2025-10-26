// server.c -- added for phase 2
// server listens for TCP socket connections from clients and executes
// shell commands received from the client using the Phase 1 shell implementation
// server captures command output and sends it back to the client

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// socket programming
#include <sys/socket.h>  // socket(), bind(), listen(), accept(), send(), recv()
#include <netinet/in.h>  // struct sockaddr_in, INADDR_ANY
#include <arpa/inet.h>   // htons()

// system includes
#include <errno.h>       // errno for error handling
#include <sys/wait.h>    // waitpid() for waiting on child processes

// need to include Phase 1 shell implementation -- already corrected the mistakes
#include "shell_utils.h"


// SERVER configuration constants
#define PORT 8080                 // server listens on port 8080
#define BUFFER_SIZE 4096          // size of buffers for receiving/sending data
#define BACKLOG 5                 // max number of pending connections in listen queue


// server display functions -- print formatted messages to server console
void print_info(const char* message);
void print_received(const char* command);
void print_executing(const char* command);
void print_output(const char* message);
void print_error(const char* message);

// socket management functions
int create_server_socket(void);
int accept_client_connection(int server_fd);

// client handling and command execution
void handle_client(int client_fd);
char* execute_command_with_capture(const char* command, int* success);


// server entry point
int main(void) {
    int server_fd;  // server socket file descriptor
    int client_fd;  // client socket file descriptor

    // create and configure server socket
    server_fd = create_server_socket();
    if (server_fd == -1) {
        fprintf(stderr, "Error -- Failed to create server socket\n");
        return EXIT_FAILURE;
    }

    // print startup message to indicate server is ready
    print_info("Server started, waiting for client connections...");

    // accept one client connection -- for now, handling one client at a time
    client_fd = accept_client_connection(server_fd);
    if (client_fd == -1) {
        close(server_fd);
        return EXIT_FAILURE;
    }

    // print client connected message
    print_info("Client connected.");

    // handle all commands from this client until disconnection
    handle_client(client_fd);

    // cleanup - close both client and server sockets
    close(client_fd);
    close(server_fd);

    return EXIT_SUCCESS;
}


// socket creation and configuration
// creates, binds, and configures a TCP server socket
// returns: server socket file descriptor on success, -1 on failure
int create_server_socket(void) {
    int server_fd;
    struct sockaddr_in server_addr;
    int opt = 1;

    // create TCP socket (AF_INET = IPv4, SOCK_STREAM = TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error: Socket creation failed");
        return -1;
    }

    // set socket option SO_REUSEADDR to allow immediate reuse of port
    // --> prevents "address already in use" errors when restarting server
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("Error: setsockopt failed");
        close(server_fd);
        return -1;
    }

    // Configure server address structure
    memset(&server_addr, 0, sizeof(server_addr));         
    server_addr.sin_family = AF_INET;                     
    server_addr.sin_addr.s_addr = INADDR_ANY;             
    server_addr.sin_port = htons(PORT);                   

    // bind socket to the configured address and port
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error: Bind failed");
        close(server_fd);
        return -1;
    }

    // start listening for incoming connections
    // BACKLOG used for --> max number of pending connections in the queue
    if (listen(server_fd, BACKLOG) == -1) {
        perror("Error: Listen failed");
        close(server_fd);
        return -1;
    }

    return server_fd;
}


// client connection handling
// accepts incoming client connections -- returns client socket fd on success, -1 on failure
int accept_client_connection(int server_fd) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    // accept incoming connection - this blocks until a client connects
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1) {
        perror("Error -- Accept failed");
        return -1;
    }

    return client_fd;
}

// command execution with output capture
// executes a shell command and captures all its output (stdout and stderr)
// integrates Phase 1 shell with Phase 2 networking
// create two pipes: one for stdout, one for stderr -- fork child process
// child --> redirect stdout/stderr to pipes
// parent --> read from pipes to capture all output, wait for child to finish
char* execute_command_with_capture(const char* command, int* success) {
    // create pipes for capturing stdout and stderr separately -- [0] = read end, [1] = write end
    int stdout_pipe[2]; 
    int stderr_pipe[2];

    // create stdout pipe
    if (pipe(stdout_pipe) == -1) {
        perror("Error: pipe creation failed for stdout");
        *success = 0;
        return NULL;
    }

    // create stderr pipe
    if (pipe(stderr_pipe) == -1) {
        perror("Error: pipe creation failed for stderr");
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        *success = 0;
        return NULL;
    }

    // fork a child process to execute the command
    pid_t pid = fork();

    if (pid == -1) {
        // fork failed
        perror("Error -- fork failed");
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        close(stderr_pipe[0]);
        close(stderr_pipe[1]);
        *success = 0;
        return NULL;
    }

    if (pid == 0) {
        // child process
        // execute the command with redirected output

        // close read ends of pipes -- child only writes to pipes
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // redirect stdout to the write end of stdout_pipe
        // anything printed to stdout goes into the pipe
        dup2(stdout_pipe[1], STDOUT_FILENO);

        // redirect stderr to the write end of stderr_pipe
        // anything printed to stderr goes into the pipe
        dup2(stderr_pipe[1], STDERR_FILENO);

        // Close the original write ends after duplication
        // file descriptors are now duplicated onto STDOUT/STDERR
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        // make a copy of the command string
        char* cmd_copy = malloc(strlen(command) + 1);
        if (cmd_copy == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        strcpy(cmd_copy, command);

        // parse the command using Phase 1 parser
        // handles: simple commands, pipes, redirections, compound commands
        pipeline_t* pipeline = parse_pipeline(cmd_copy);
        free(cmd_copy);

        if (pipeline == NULL) {
            // Parsing failed - invalid command syntax
            fprintf(stderr, "Error -- Invalid command: %s\n", command);
            exit(EXIT_FAILURE);
        }

        // execute the parsed pipeline using Phase 1 features
        int status = execute_pipeline(pipeline);

        // clean up allocated memory
        free_pipeline(pipeline);

        // exit with the command's exit status
        // Exit code 0 -> success, != 0 -> error
        exit(status);

    } else {
        // parent
        // read the captured output from pipes

        // close write ends of pipes (parent only reads from pipes)
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        // buffers for reading from pipes
        char stdout_buffer[BUFFER_SIZE];
        char stderr_buffer[BUFFER_SIZE];
        ssize_t stdout_bytes = 0;
        ssize_t stderr_bytes = 0;

        // allocate initial memory for combined output
        // dynamically grow this buffer as needed
        size_t total_size = BUFFER_SIZE * 2;
        char* output = malloc(total_size);
        if (output == NULL) {
            perror("Error: malloc failed for output");
            close(stdout_pipe[0]);
            close(stderr_pipe[0]);
            waitpid(pid, NULL, 0);
            *success = 0;
            return NULL;
        }
        output[0] = '\0';  // start with empty string
        size_t output_len = 0;

        // read all data from stdout pipe
        // loop continues until pipe is closed
        while ((stdout_bytes = read(stdout_pipe[0], stdout_buffer, BUFFER_SIZE - 1)) > 0) {
            stdout_buffer[stdout_bytes] = '\0';  // null-terminate the chunk

            // dynamically resize output buffer if needed
            // double the size when we run out of space
            while (output_len + stdout_bytes + 1 > total_size) {
                total_size *= 2;
                char* new_output = realloc(output, total_size);
                if (new_output == NULL) {
                    perror("Error: realloc failed");
                    free(output);
                    close(stdout_pipe[0]);
                    close(stderr_pipe[0]);
                    waitpid(pid, NULL, 0);
                    *success = 0;
                    return NULL;
                }
                output = new_output;
            }

            // append this chunk to the output string
            strcat(output, stdout_buffer);
            output_len += stdout_bytes;
        }

        // read all data from stderr pipe
        // Same process as stdout - read until pipe closes
        while ((stderr_bytes = read(stderr_pipe[0], stderr_buffer, BUFFER_SIZE - 1)) > 0) {
            stderr_buffer[stderr_bytes] = '\0';

            // dynamically resize in case needed
            while (output_len + stderr_bytes + 1 > total_size) {
                total_size *= 2;
                char* new_output = realloc(output, total_size);
                if (new_output == NULL) {
                    perror("Error: realloc failed");
                    free(output);
                    close(stdout_pipe[0]);
                    close(stderr_pipe[0]);
                    waitpid(pid, NULL, 0);
                    *success = 0;
                    return NULL;
                }
                output = new_output;
            }

            // append stderr to output (after stdout)
            strcat(output, stderr_buffer);
            output_len += stderr_bytes;
        }

        // close read ends of pipes
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        // wait for child process to finish and get its exit status
        int status;
        waitpid(pid, &status, 0);

        // determine if command executed successfully based on exit status
        if (WIFEXITED(status)) {
            int exit_status = WEXITSTATUS(status);
            if (exit_status == 0) {
                // command executed successfully
                *success = 1;
            } else {
                // command executed but returned error
                // check if it's a "command not found" error by looking at output
                if (strstr(output, "Command not found") != NULL ||
                    strstr(output, "not found") != NULL) {
                    *success = 0;  // command doesn't exist
                } else {
                    *success = 1;  // command ran but had error output
                }
            }
        } else {
            // child process did not exit normally
            *success = 0;
        }

        // return the captured output
        // caller responsible for freeing this memory
        return output;
    }

    // should never reach here
    return NULL;
}


// client command handling loop
// receives commands from client via socket, executes each command, sends the captured output back to client, displays messages
// continue until client disconnects or sends "exit" command
void handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    // continues until client disconnects or sends "exit"
    while (1) {
        // clear buffer before receiving new command
        memset(buffer, 0, BUFFER_SIZE);

        // receive command from client
        // recv() blocks until data is available or connection closes
        bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        // check for errors or connection closed
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                // client closed connection gracefully
                printf("[INFO] Client disconnected.\n");
            } else {
                // recv() error
                perror("Error: recv failed");
            }
            break;
        }

        // null-terminate the received data to make it a valid C string
        buffer[bytes_received] = '\0';

        // remove trailing newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // display received command on server console with formatting
        print_received(buffer);

        // check for exit command
        if (strcmp(buffer, "exit") == 0) {
            printf("[INFO] Client requested exit.\n");
            // send empty response to acknowledge exit
            const char* exit_msg = "";
            send(client_fd, exit_msg, strlen(exit_msg), 0);
            break;  // exit the command loop
        }

        // display executing message on server console
        print_executing(buffer);

        // execute the command and capture its output
        int success = 0;
        char* output = execute_command_with_capture(buffer, &success);

        if (output != NULL) {
            // command execution completed (successfully or with error)

            if (success) {
                // command executed successfully
                print_output("Sending output to client:");

                // display the actual output on server console
                printf("%s", output);
                // add newline if output doesn't end with one
                if (strlen(output) > 0 && output[strlen(output) - 1] != '\n') {
                    printf("\n");
                }
            } else {
                // command not found or execution failed
                // display error message on server console with formatting
                printf("[ERROR] Command not found: \"%s\"\n", buffer);
                print_output("Sending error message to client:");
                printf("%s", output);
                if (strlen(output) > 0 && output[strlen(output) - 1] != '\n') {
                    printf("\n");
                }
            }

            // send the captured output (or error message) back to client
            // if output is empty (e.g., redirected to file), send newline so client knows command completed
            if (strlen(output) == 0) {
                ssize_t bytes_sent = send(client_fd, "\n", 1, 0);
                if (bytes_sent == -1) {
                    perror("Error: send failed");
                    free(output);
                    break;
                }
            } else {
                ssize_t bytes_sent = send(client_fd, output, strlen(output), 0);
                if (bytes_sent == -1) {
                    perror("Error: send failed");
                    free(output);
                    break;
                }
            }

            // free the dynamically allocated output string
            free(output);

        } else {
            // memory allocation failed or other critical error
            const char* error_msg = "Error: Server failed to execute command\n";
            print_output("Sending error message to client:");
            printf("%s", error_msg);
            send(client_fd, error_msg, strlen(error_msg), 0);
        }
    }
}

// server console output formatting functions

// format and display messages on the server console
// match the exact format shown in the assignment screenshots
void print_info(const char* message) {
    printf("[INFO] %s\n", message);
    fflush(stdout);  // ensure output is displayed immediately
}

void print_received(const char* command) {
    printf("[RECEIVED] Received command: \"%s\" from client.\n", command);
    fflush(stdout);
}

void print_executing(const char* command) {
    printf("[EXECUTING] Executing command: \"%s\"\n", command);
    fflush(stdout);
}
void print_output(const char* message) {
    printf("[OUTPUT] %s\n", message);
    fflush(stdout);
}

void print_error(const char* message) {
    printf("[ERROR] %s\n", message);
    fflush(stdout);
}
