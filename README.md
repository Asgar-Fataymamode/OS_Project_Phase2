# Phase 2: Remote Shell via Socket Communication

## Overview

This project extends the Phase 1 command-line shell to support remote access through TCP socket communication. A client program connects to a server, which executes commands using the Phase 1 shell implementation and returns results to the client.

## System Architecture

The system consists of three main components:

1. **Client** (`client.c`): Presents a shell interface to users, sends commands to server, displays results
2. **Server** (`server.c`): Accepts client connections, executes commands using Phase 1 shell, returns output
3. **Shell Engine** (`shell_utils.c/h`): Phase 1 implementation handling command parsing, pipes, and redirections

## Compilation

### Build All Components

```bash
make all
```

This compiles:
- `myshell`: Phase 1 local shell
- `server`: Phase 2 server
- `client`: Phase 2 client

### Build Individual Components

```bash
make server    # Build only server
make client    # Build only client
make myshell   # Build only Phase 1 shell
```

### Clean Build Artifacts

```bash
make clean     # Remove all object files and executables
make rebuild   # Clean and build from scratch
```

## Execution

### Starting the Server

Open a terminal and run:

```bash
./server
```

Expected output:
```
[INFO] Server started, waiting for client connections...
```

The server listens on port 8080 and waits for a client connection.

### Starting the Client

In a separate terminal, run:

```bash
./client
```

For remote servers:
```bash
./client <server_ip>           # Connect to specific IP
./client <server_ip> <port>    # Connect to specific IP and port
```

Examples:
```bash
./client                        # Connect to localhost:8080
./client 192.168.1.100         # Connect to remote server
./client localhost 9090        # Connect to custom port
```

### Using the Shell

Once connected, use the client like a normal shell:

```bash
$ pwd
/path/to/directory
$ ls
file1.txt
file2.txt
$ ls | grep txt
file1.txt
file2.txt
$ echo Hello World
Hello World
$ exit
```

## Features

### Phase 1 Features (Preserved)

All Phase 1 functionality works through the client-server system:

- Simple commands: `pwd`, `ls`, `echo`
- Commands with arguments: `ls -l`, `ps aux`
- Pipes: `ls | grep pattern`, `cat file | wc -l`
- Input redirection: `cat < input.txt`
- Output redirection: `echo text > output.txt`
- Error redirection: `command 2> error.log`
- Complex pipelines: `cat file | grep pattern | sort | uniq`
- Built-in commands: `echo`

### Phase 2 Features (New)

- TCP socket communication between client and server
- Remote command execution
- Multi-line output handling
- Connection error detection and reporting
- Graceful exit handling
- Server logging with formatted output

## Protocol Specification

### Communication Details

- **Protocol**: TCP (connection-oriented)
- **Port**: 8080 (default)
- **Buffer Size**: 4096 bytes
- **Message Format**: Plain text with newline termination

### Message Flow

1. Client sends command with newline: `<command>\n`
2. Server receives, executes command, captures output
3. Server sends complete output (may span multiple packets)
4. Client receives and displays output
5. Process repeats until exit command

### Exit Procedure

1. Client sends: `exit\n`
2. Server acknowledges with empty response
3. Both close connection gracefully

## File Structure

```
OS_Project_Phase2/
├── client.c                # Client implementation (Person B)
├── server.c                # Server implementation (Person A)
├── shell_utils.c           # Phase 1 shell logic
├── shell_utils.h           # Phase 1 header file
├── myshell.c               # Phase 1 local shell
├── Makefile                # Build system
├── protocol.md             # Communication protocol documentation
├── GRADING_RUBRIC_REVIEW.md # Rubric compliance analysis
└── tests/                  # Test files
    ├── testfile1.txt
    └── testfile2.txt
```

## Error Handling

### Client-Side Errors

**Cannot Connect to Server**
```
Error: Cannot connect to server at 127.0.0.1:8080
Please ensure the server is running and the IP/port are correct.
To start the server: ./server
```
Resolution: Start the server before running client

**Connection Lost During Operation**
```
Error: Connection to server lost.
The server may have crashed or the network connection was interrupted.
```
Resolution: Restart server and reconnect client

**Invalid Port Number**
```
Error: Invalid port number. Using default port 8080
```
Resolution: Provide valid port (1-65535)

### Server-Side Errors

**Port Already in Use**
```
Error: Bind failed: Address already in use
```
Resolution: Kill existing server process or use different port

**Command Not Found**
```
[ERROR] Command not found: "unknowncmd"
```
Server displays error and sends message to client

## Testing

### Automated Testing

Run comprehensive test suite:

```bash
./test_client.sh
```

Tests include:
- Simple commands
- Commands with arguments
- Phase 1 pipes
- Phase 1 redirections
- Error handling
- Edge cases

### Manual Testing

1. Start server in one terminal: `./server`
2. Start client in another: `./client`
3. Test various commands:
   - `pwd` - Show current directory
   - `ls` - List files
   - `ls | grep test` - Test pipes
   - `cat < input` - Test input redirection
   - `unknowncmd` - Test error handling
   - `exit` - Test clean exit

## Implementation Notes

### Socket Functions Used

**Client:**
- `socket()`: Create TCP socket
- `connect()`: Connect to server
- `send()`: Send commands
- `recv()`: Receive output
- `close()`: Close connection

**Server:**
- `socket()`: Create TCP socket
- `bind()`: Bind to port 8080
- `listen()`: Listen for connections
- `accept()`: Accept client connections
- `send()`: Send output
- `recv()`: Receive commands
- `close()`: Close connections

### Design Decisions

1. **Single Client per Server**: Server handles one client at a time for simplicity
2. **Blocking I/O**: Appropriate for single-user client-server model
3. **Buffer Size**: 4096 bytes balances memory usage and large output handling
4. **Protocol Simplicity**: Plain text communication for easy debugging and implementation
5. **Error Verbosity**: Detailed error messages aid troubleshooting

### Code Organization

- **Modular Design**: Separate functions for socket management, command handling, error reporting
- **Clear Separation**: Client and server in separate files with no shared code
- **Reusability**: Phase 1 shell code reused without modification
- **Maintainability**: Extensive comments and clear variable names

## Troubleshooting

### Server Won't Start

**Problem**: "Bind failed: Address already in use"

**Solution**:
```bash
# Find process using port 8080
lsof -i :8080
# Or
netstat -an | grep 8080

# Kill the process
kill <PID>
```

### Client Can't Connect

**Problem**: "Cannot connect to server"

**Checklist**:
1. Is server running? Check with `ps aux | grep server`
2. Correct IP address? Try `./client 127.0.0.1`
3. Correct port? Default is 8080
4. Firewall blocking? Check firewall settings

### Commands Not Working

**Problem**: Commands work locally but fail remotely

**Check**:
1. File paths relative to server's working directory
2. Permissions on server filesystem
3. Commands available on server (check PATH)
4. Server-side error messages in server output

## Additional Information

### Requirements

- GCC compiler or compatible
- POSIX-compliant operating system (Linux, macOS, Unix)
- Network connectivity for remote access

### Compilation Flags

```
-Wall          # Enable all warnings
-Wextra        # Extra warnings
-std=c99       # C99 standard
-g             # Debug symbols
-pedantic      # Strict standard compliance
```

### Performance Considerations

- Buffer size optimized for typical command output
- No unnecessary data copying
- Efficient error checking with minimal overhead
- Reuses buffers to avoid repeated allocation

### Security Notes

This is an educational implementation with no security features:
- No authentication
- No encryption
- No input sanitization beyond Phase 1 shell
- Should only be used in trusted network environments

For production use, consider:
- SSL/TLS encryption
- User authentication
- Input validation and sanitization
- Rate limiting and access control

## Contact Information

For questions about:
- **Client implementation**: Person B
- **Server implementation**: Person A  
- **Protocol specification**: See `protocol.md`
- **Phase 1 shell**: See Phase 1 documentation

## Version History

- **Phase 1**: Local command-line shell with pipes and redirections
- **Phase 2**: Added client-server architecture with socket communication

## License

Academic project for Operating Systems course. All rights reserved.

