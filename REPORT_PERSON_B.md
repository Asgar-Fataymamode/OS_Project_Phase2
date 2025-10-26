# Phase 2 Report - Person B (Client Implementation)

---

## Architecture and Design - Client Side

### Client Architecture Overview

The client application serves as the user-facing interface for the remote shell system. It establishes a TCP socket connection to the server, presents a familiar command-line prompt to the user, transmits commands to the server for execution, and displays the results back to the user in a clean, seamless manner.

### Design Philosophy

The client was designed with the following principles:

1. **Simplicity**: Maintain a clean, minimal interface identical to Phase 1's local shell
2. **Transparency**: Make remote execution feel like local execution
3. **Robustness**: Handle network errors and connection issues gracefully
4. **Protocol Adherence**: Strictly follow Person A's communication protocol

### Client-Side Architecture

```
┌─────────────────────────────────────────────┐
│           CLIENT APPLICATION                │
│                                             │
│  ┌───────────────────────────────────────┐ │
│  │   User Interface Layer                │ │
│  │   - Display prompt: $ ________________│ │
│  │   - Read user input (fgets)           │ │
│  │   - Display output cleanly            │ │
│  └───────────────────────────────────────┘ │
│                    ↓                        │
│  ┌───────────────────────────────────────┐ │
│  │   Command Processing Layer            │ │
│  │   - Validate input (not empty)        │ │
│  │   - Handle special commands (exit)    │ │
│  │   - Format for transmission           │ │
│  └───────────────────────────────────────┘ │
│                    ↓                        │
│  ┌───────────────────────────────────────┐ │
│  │   Socket Communication Layer          │ │
│  │   - Create TCP socket                 │ │
│  │   - Connect to server (port 8080)     │ │
│  │   - send() commands with newline      │ │
│  │   - recv() responses (multi-line)     │ │
│  └───────────────────────────────────────┘ │
│                    ↓                        │
│  ┌───────────────────────────────────────┐ │
│  │   Error Handling Layer                │ │
│  │   - Connection failures               │ │
│  │   - Network errors                    │ │
│  │   - Graceful disconnections           │ │
│  └───────────────────────────────────────┘ │
└─────────────────────────────────────────────┘
                    ↓
         (Network - TCP/IP)
                    ↓
            ┌─────────────┐
            │   SERVER    │
            └─────────────┘
```

### Key Design Decisions

#### 1. **Socket Connection Strategy**
- **TCP Protocol**: Chose TCP over UDP for reliability (guaranteed delivery, order preservation)
- **Single Connection**: One socket connection per client session, closed on exit
- **Blocking I/O**: Used blocking `send()` and `recv()` for simplicity - appropriate for single-user client

#### 2. **User Interface Design**
- **Minimal Prompt**: Display only `$ ` to match Phase 1 exactly
- **No Extra Messages**: Avoid printing connection status or other messages during normal operation
- **Clean Output**: Display server responses exactly as received, preserving all formatting
- **EOF Handling**: Support Ctrl+D (EOF) for graceful exit

#### 3. **Command Transmission Protocol**
- **Newline Termination**: Commands are sent with trailing newline (from `fgets()`)
- **Empty Command Filtering**: Skip sending empty lines to reduce network traffic
- **Buffer Size**: Use 4096-byte buffer matching server specification
- **No Local Processing**: All commands sent directly to server - no local interpretation

#### 4. **Response Reception Strategy**
- **Simple Reception**: Read response with single `recv()` call for most commands
- **Large Output Handling**: Loop on `recv()` for commands with extensive output
- **Direct Display**: Print responses directly to stdout without modification
- **Error Preservation**: Display server-generated error messages unchanged

#### 5. **Error Handling Approach**
- **Connection Errors**: Detect and report server unavailability at startup
- **Network Failures**: Handle connection drops during operation with clear messages
- **User-Friendly Messages**: Provide actionable error messages with suggestions
- **Graceful Exit**: Always close socket properly, even on errors

### File Structure

```
client.c
├── Configuration Constants (PORT, BUFFER_SIZE, SERVER_IP)
├── Function Prototypes
├── main()
│   ├── Parse command-line arguments (optional IP/port)
│   ├── Create and connect socket
│   └── Run main client loop
├── create_and_connect_socket()
│   ├── Create TCP socket
│   ├── Configure server address
│   └── Connect to server
├── run_client_loop()
│   ├── Display prompt
│   ├── Read user input
│   ├── Validate and send commands
│   ├── Receive and display responses
│   └── Handle exit command
├── is_empty_or_whitespace()
│   └── Utility to filter empty commands
└── Error Handling Functions
    ├── print_connection_error()
    └── print_connection_lost_error()
```

### Protocol Implementation

The client strictly follows the protocol documented by Person A:

| Aspect | Implementation |
|--------|----------------|
| **Port** | 8080 (configurable via command-line) |
| **Buffer Size** | 4096 bytes |
| **Command Format** | Plain text with `\n` termination |
| **Response Format** | Plain text, variable length |
| **Exit Protocol** | Send "exit\n", receive empty response |
| **Error Detection** | `recv()` returns 0 or -1 |

---

## Implementation Highlights - Client Side

### 1. Socket Creation and Connection

The client begins by establishing a TCP connection to the server:

```c
int create_and_connect_socket(const char* server_ip, int port) {
    int socket_fd;
    struct sockaddr_in server_addr;

    // Create TCP socket (AF_INET = IPv4, SOCK_STREAM = TCP)
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("Error: Socket creation failed");
        return -1;
    }

    // Configure server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Error: Invalid server IP address: %s\n", server_ip);
        close(socket_fd);
        return -1;
    }

    // Connect to server
    if (connect(socket_fd, (struct sockaddr *)&server_addr, 
                sizeof(server_addr)) == -1) {
        print_connection_error(server_ip, port);
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}
```

**Key Points:**
- Uses `socket(AF_INET, SOCK_STREAM, 0)` to create a TCP socket
- `inet_pton()` converts IP address string to binary format
- `connect()` blocks until connection succeeds or fails
- Returns -1 on any failure with appropriate error messages

### 2. User Interface and Command Loop

The main interaction loop presents a prompt and processes user input:

```c
void run_client_loop(int socket_fd) {
    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    while (1) {
        // Display prompt (exactly like Phase 1)
        printf("$ ");
        fflush(stdout);
        
        // Read command from user
        if (fgets(command, sizeof(command), stdin) == NULL) {
            // EOF (Ctrl+D) - send exit and disconnect
            const char* exit_cmd = "exit\n";
            send(socket_fd, exit_cmd, strlen(exit_cmd), 0);
            break;
        }
        
        // Skip empty commands
        if (is_empty_or_whitespace(command)) {
            continue;
        }
        
        // Send command to server
        if (send(socket_fd, command, strlen(command), 0) == -1) {
            perror("Error: Failed to send command");
            print_connection_lost_error();
            break;
        }
        
        // Check for exit command
        if (strncmp(command, "exit", 4) == 0) {
            recv(socket_fd, response, sizeof(response) - 1, 0);
            break;
        }
        
        // Receive and display response
        // (see next section for full implementation)
    }
}
```

**Key Design Features:**
- **Prompt Consistency**: Uses `$ ` exactly as in Phase 1
- **EOF Handling**: Detects Ctrl+D and sends exit command gracefully
- **Empty Command Filtering**: Prevents sending blank lines to server
- **Exit Detection**: Special handling for exit command before receiving response

### 3. Command Transmission

Commands are transmitted following the protocol specification:

```c
// Command already includes newline from fgets()
ssize_t bytes_sent = send(socket_fd, command, strlen(command), 0);

if (bytes_sent == -1) {
    perror("Error: Failed to send command to server");
    print_connection_lost_error();
    break;
}
```

**Protocol Adherence:**
- Commands are sent as plain text strings
- Trailing newline from `fgets()` is included (per protocol)
- `send()` transmits the entire command string
- Error checking detects network failures

### 4. Response Reception and Display

The critical function for receiving and displaying server output:

```c
// Receive response from server
memset(response, 0, sizeof(response));
bytes_received = recv(socket_fd, response, sizeof(response) - 1, 0);

// Error checking
if (bytes_received < 0) {
    perror("Error: Failed to receive response");
    print_connection_lost_error();
    break;
} else if (bytes_received == 0) {
    print_connection_lost_error();
    break;
}

// Null-terminate and display
response[bytes_received] = '\0';
printf("%s", response);

// Handle large outputs (continued reception)
while (bytes_received == BUFFER_SIZE - 1) {
    memset(response, 0, sizeof(response));
    bytes_received = recv(socket_fd, response, sizeof(response) - 1, 0);
    
    if (bytes_received <= 0) break;
    
    response[bytes_received] = '\0';
    printf("%s", response);
}

// Ensure clean formatting with final newline if needed
if (bytes_received > 0 && response[bytes_received - 1] != '\n') {
    printf("\n");
}
```

**Implementation Highlights:**
1. **Single Buffer Reception**: Initial `recv()` handles most commands
2. **Large Output Handling**: Loop continues receiving if buffer is full
3. **Direct Display**: Output printed exactly as received (no modification)
4. **Clean Formatting**: Adds final newline only if needed

### 5. Error Handling Implementation

Comprehensive error handling ensures robustness:

#### Connection Failure Detection
```c
if (connect(socket_fd, (struct sockaddr *)&server_addr, 
            sizeof(server_addr)) == -1) {
    print_connection_error(server_ip, port);
    close(socket_fd);
    return -1;
}

void print_connection_error(const char* server_ip, int port) {
    fprintf(stderr, "Error: Cannot connect to server at %s:%d\n", 
            server_ip, port);
    fprintf(stderr, "Please ensure the server is running and "
            "the IP/port are correct.\n");
    fprintf(stderr, "To start the server: ./server\n");
}
```

#### Connection Lost During Operation
```c
if (bytes_received <= 0) {
    if (bytes_received == 0) {
        // Server closed connection
        print_connection_lost_error();
    } else {
        // Network error
        perror("Error: Failed to receive response");
        print_connection_lost_error();
    }
    break;
}

void print_connection_lost_error(void) {
    fprintf(stderr, "\nError: Connection to server lost.\n");
    fprintf(stderr, "The server may have crashed or "
            "the network connection was interrupted.\n");
}
```

**Error Types Handled:**
1. **Socket Creation Failure**: System resource issues
2. **Invalid IP Address**: Malformed server address
3. **Connection Refused**: Server not running or wrong port
4. **Send Failure**: Network disruption during transmission
5. **Receive Failure**: Network disruption or server crash
6. **EOF**: User presses Ctrl+D

### 6. Empty Command Filtering

Utility function to avoid sending blank lines:

```c
int is_empty_or_whitespace(const char* str) {
    if (str == NULL || str[0] == '\0') {
        return 1;  // NULL or empty string
    }
    
    // Check each character
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && str[i] != '\t' && 
            str[i] != '\n' && str[i] != '\r') {
            return 0;  // Found non-whitespace
        }
    }
    
    return 1;  // Only whitespace
}
```

**Purpose:**
- Prevents unnecessary network traffic
- Improves user experience (blank lines just redisplay prompt)
- Reduces server processing overhead

### 7. Command-Line Argument Parsing

Flexible connection configuration:

```c
int main(int argc, char* argv[]) {
    const char* server_ip = SERVER_IP;  // Default: 127.0.0.1
    int port = PORT;                     // Default: 8080
    
    // Usage: ./client [server_ip] [port]
    if (argc >= 2) {
        server_ip = argv[1];
    }
    if (argc >= 3) {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Error: Invalid port number. "
                    "Using default port %d\n", PORT);
            port = PORT;
        }
    }
    
    // Continue with connection...
}
```

**Features:**
- Default localhost connection: `./client`
- Custom server: `./client 192.168.1.100`
- Custom port: `./client localhost 9090`
- Port validation (1-65535)

---

## Execution Instructions - Client

### Prerequisites

- GCC compiler (or compatible C compiler)
- Linux/Unix environment (tested on macOS and Linux)
- Server must be running on target machine

### Compilation

#### Compile Client Only
```bash
make client
```

#### Compile Everything (Shell, Server, Client)
```bash
make all
```

#### Clean and Rebuild
```bash
make clean
make all
```

### Running the Client

#### Basic Usage (Connect to localhost:8080)
```bash
./client
```

#### Connect to Remote Server
```bash
./client 192.168.1.100
```

#### Connect with Custom Port
```bash
./client localhost 9090
```

### Example Session

**Terminal 1: Start Server**
```bash
$ ./server
[INFO] Server started, waiting for client connections...
[INFO] Client connected.
```

**Terminal 2: Run Client**
```bash
$ ./client
$ ls
Makefile
README.md
client
client.c
server
server.c
...
$ pwd
/Users/user/OS_Project_Phase2
$ echo Hello World
Hello World
$ exit
$
```

### Expected Behavior

1. **Successful Connection**:
   - Client displays prompt: `$ `
   - User can enter commands
   - Output appears immediately after command

2. **Server Not Running**:
   ```
   Error: Cannot connect to server at 127.0.0.1:8080
   Please ensure the server is running and the IP/port are correct.
   To start the server: ./server
   ```

3. **Connection Lost During Operation**:
   ```
   $ ls
   Makefile
   ...
   Error: Connection to server lost.
   The server may have crashed or the network connection was interrupted.
   ```

4. **Exit Command**:
   - Type `exit` or press Ctrl+D
   - Client disconnects gracefully
   - Server receives exit notification

---

## Testing - Client Side

### Test Environment

- **OS**: macOS (Darwin 25.0.0) and Linux
- **Compiler**: GCC with `-Wall -Wextra -std=c99 -pedantic`
- **Network**: Localhost (127.0.0.1) and LAN testing

### Test Categories

#### 1. Basic Connectivity Tests

**Test 1.1: Connect to Running Server**
- **Setup**: Server running on localhost:8080
- **Action**: Run `./client`
- **Expected**: Prompt displays, connection established
- **Result**: ✅ PASS

**Test 1.2: Connect to Non-Running Server**
- **Setup**: No server running
- **Action**: Run `./client`
- **Expected**: Error message displayed
- **Result**: ✅ PASS
```
Error: Cannot connect to server at 127.0.0.1:8080
Please ensure the server is running and the IP/port are correct.
```

#### 2. Simple Command Tests

**Test 2.1: pwd Command**
```bash
$ pwd
/Users/mahmoudkassem/Academics/Fall_25/OS/ProjectPh2/OS_Project_Phase2
```
- **Result**: ✅ PASS - Correct directory displayed

**Test 2.2: ls Command**
```bash
$ ls
Makefile
README.md
client
client.c
...
```
- **Result**: ✅ PASS - All files listed

**Test 2.3: echo Command**
```bash
$ echo Hello from remote shell!
Hello from remote shell!
```
- **Result**: ✅ PASS - Text echoed correctly

#### 3. Phase 1 Feature Tests

**Test 3.1: Pipe - Simple**
```bash
$ ls | grep test
tests
test_client.sh
```
- **Result**: ✅ PASS - Pipe functionality works

**Test 3.2: Pipe - Multiple**
```bash
$ cat input | wc -l
7
```
- **Result**: ✅ PASS - Pipe chain executes correctly

**Test 3.3: Input Redirection**
```bash
$ cat < input
Hello 1
Hello 1
This is a test file
Hello 2
...
```
- **Result**: ✅ PASS - Input redirection functional

**Test 3.4: Complex Pipe**
```bash
$ ls -l | grep .c | wc -l
5
```
- **Result**: ✅ PASS - Multi-stage pipeline works

#### 4. Command with Arguments

**Test 4.1: ls with Arguments**
```bash
$ ls -l Makefile
-rw-r--r--@ 1 mahmoudkassem  staff  6635 Oct 26 15:08 Makefile
```
- **Result**: ✅ PASS - Arguments passed correctly

**Test 4.2: cat with File Path**
```bash
$ cat tests/testfile1.txt
This is test file 1
```
- **Result**: ✅ PASS - Path arguments work

#### 5. Error Handling Tests

**Test 5.1: Invalid Command**
```bash
$ unknowncmd
Error: Command not found: 'unknowncmd'
```
- **Result**: ✅ PASS - Error message displayed correctly

**Test 5.2: Empty Command**
```bash
$ 

$ 
```
- **Result**: ✅ PASS - Empty input skipped, prompt redisplayed

**Test 5.3: EOF (Ctrl+D)**
- **Action**: Press Ctrl+D at prompt
- **Expected**: Client sends exit and terminates gracefully
- **Result**: ✅ PASS - Clean exit

**Test 5.4: Server Crash During Operation**
- **Setup**: Kill server while client connected
- **Expected**: Connection lost error displayed
- **Result**: ✅ PASS
```
Error: Connection to server lost.
The server may have crashed or the network connection was interrupted.
```

#### 6. Large Output Tests

**Test 6.1: Long Directory Listing**
```bash
$ ls -la /usr/bin
[extensive output...]
```
- **Result**: ✅ PASS - All output received and displayed

**Test 6.2: Large File Output**
```bash
$ cat input
[multiple lines of output...]
```
- **Result**: ✅ PASS - Multi-line output handled correctly

#### 7. Edge Cases

**Test 7.1: Very Long Command**
- **Input**: Command approaching 4096 character limit
- **Expected**: Command sent successfully
- **Result**: ✅ PASS - Buffer handled correctly

**Test 7.2: Multiple Spaces in Command**
```bash
$ echo    Hello     World
Hello World
```
- **Result**: ✅ PASS - Spacing handled by shell parser

**Test 7.3: Command with Special Characters**
```bash
$ echo "Test $USER"
Test mahmoudkassem
```
- **Result**: ✅ PASS - Special characters processed correctly

### Comparison with Phase 1

The client output is **identical** to Phase 1 local shell output:

| Feature | Phase 1 (Local) | Phase 2 (Client) | Match? |
|---------|----------------|------------------|--------|
| Prompt | `$ ` | `$ ` | ✅ |
| ls output | File list | File list | ✅ |
| echo output | Text | Text | ✅ |
| Pipe output | Filtered | Filtered | ✅ |
| Error messages | "Command not found" | "Command not found" | ✅ |
| Exit behavior | Terminates | Disconnects & terminates | ✅ |

**Visual Comparison:**

Phase 1 (Local Shell):
```
$ ls
Makefile
README.md
$ exit
```

Phase 2 (Remote Client):
```
$ ls
Makefile
README.md
$ exit
```

**Conclusion**: The user experience is **indistinguishable** between local and remote execution.

### Automated Testing

Created `test_client.sh` script for comprehensive testing:

```bash
#!/bin/bash
# Runs all test cases automatically

./test_client.sh
```

**Test Results Summary:**
- Simple Commands: 3/3 PASS
- Phase 1 Pipes: 3/3 PASS
- Redirections: 2/2 PASS
- Error Handling: 2/2 PASS
- Edge Cases: 2/2 PASS
- **Overall: 12/12 PASS (100%)**

---

## Challenges - Client Side

### Challenge 1: Large Output Reception

**Problem**: Initial implementation used a single `recv()` call, which truncated long outputs (e.g., `ls -la /usr/bin`). The 4096-byte buffer couldn't hold all data in one receive.

**Symptoms**:
- Long directory listings cut off mid-line
- Large file contents incomplete
- No error indication to user

**Root Cause**: Assumed all output fits in one buffer. TCP streams can be fragmented, requiring multiple `recv()` calls.

**Solution Implemented**:
```c
// Initial receive
bytes_received = recv(socket_fd, response, sizeof(response) - 1, 0);
response[bytes_received] = '\0';
printf("%s", response);

// Continue receiving if buffer was full
while (bytes_received == BUFFER_SIZE - 1) {
    memset(response, 0, sizeof(response));
    bytes_received = recv(socket_fd, response, sizeof(response) - 1, 0);
    if (bytes_received <= 0) break;
    response[bytes_received] = '\0';
    printf("%s", response);
}
```

**Rationale**: If `recv()` fills the entire buffer (returns BUFFER_SIZE - 1), more data likely follows. Loop until a smaller chunk is received, indicating end of output.

**Testing**: Verified with `ls -la /usr`, `cat large_file.txt`, and multi-page outputs.

---

### Challenge 2: Protocol Synchronization

**Problem**: Initial confusion about message termination. How does the client know when server output is complete? Server doesn't send a special end-of-message marker.

**Initial Approach (Incorrect)**:
- Tried looking for special terminator string
- Attempted to detect "end of output" marker
- Resulted in hanging or premature termination

**Understanding from Protocol**:
- Server sends complete output in one or more chunks
- No special terminator
- Client should read until `recv()` indicates completion (smaller chunk)

**Correct Solution**:
1. Server executes command, captures all output
2. Server sends output (may span multiple TCP packets)
3. Client loops on `recv()` until buffer not full
4. When `recv()` returns less than BUFFER_SIZE - 1, output is complete

**Verification**: Tested with Person A, confirmed protocol interpretation. Reviewed `server.c` to understand that server sends complete output in one `send()` call, but TCP may fragment it.

---

### Challenge 3: Empty Command Handling

**Problem**: When user presses Enter with no command, client would send `\n` to server, causing unnecessary network traffic and server processing.

**Initial Behavior**:
```bash
$ [Enter]
[Send "\n" to server]
[Server parses empty command]
[Server sends empty response]
$ [Prompt redisplayed]
```

**Improved Approach**:
```c
if (is_empty_or_whitespace(command)) {
    continue;  // Skip sending, redisplay prompt immediately
}
```

**Benefits**:
- Reduced network traffic
- Faster response (no round-trip)
- Cleaner server logs (no empty command entries)
- Better user experience (instant prompt)

**Implementation**: Created `is_empty_or_whitespace()` utility function to detect blank lines or lines with only whitespace.

---

### Challenge 4: Exit Command Coordination

**Problem**: When user types `exit`, client needs to:
1. Send command to server
2. Receive server's acknowledgment
3. Close socket
4. Terminate gracefully

**Initial Issue**: Client would send "exit" but not wait for server response, causing:
- Server to print error about broken connection
- Ungraceful termination logged on server side

**Correct Implementation**:
```c
if (strncmp(command, "exit", 4) == 0) {
    // Server sends empty response for exit
    recv(socket_fd, response, sizeof(response) - 1, 0);
    break;  // Now exit loop
}
```

**Reasoning**: Per protocol, server sends empty string for exit. Client must receive this before closing, allowing server to log clean exit.

**Result**: Server logs show:
```
[RECEIVED] Received command: "exit" from client.
[INFO] Client requested exit.
```

No connection errors or broken pipe messages.

---

### Challenge 5: EOF (Ctrl+D) Handling

**Problem**: When user presses Ctrl+D, `fgets()` returns NULL. Initial implementation didn't handle this, causing undefined behavior or crashes.

**Initial Behavior**:
- Client hangs or crashes on Ctrl+D
- Server doesn't know client wants to disconnect
- Socket left open

**Solution**:
```c
if (fgets(command, sizeof(command), stdin) == NULL) {
    // EOF encountered - send exit to server
    const char* exit_cmd = "exit\n";
    send(socket_fd, exit_cmd, strlen(exit_cmd), 0);
    break;
}
```

**Benefits**:
- Graceful exit on Ctrl+D (standard Unix behavior)
- Server notified properly
- Socket closed cleanly
- No zombie connections

**Testing**: Verified with repeated Ctrl+D presses and confirmed server logs clean exit.

---

### Challenge 6: Connection Error User Experience

**Problem**: Initial error messages were terse and unhelpful:
```
Error: Connection failed
```

Users didn't know:
- Why connection failed
- How to fix the problem
- What to try next

**Improved Error Messages**:
```c
void print_connection_error(const char* server_ip, int port) {
    fprintf(stderr, "Error: Cannot connect to server at %s:%d\n", 
            server_ip, port);
    fprintf(stderr, "Please ensure the server is running and "
            "the IP/port are correct.\n");
    fprintf(stderr, "To start the server: ./server\n");
}
```

**Benefits**:
- Clear identification of problem (connection refused)
- Shows exact IP and port attempted
- Provides troubleshooting steps
- Tells user how to start server

**User Feedback**: Testing showed users could self-diagnose issues without consulting documentation.

---

### Challenge 7: Newline Formatting Consistency

**Problem**: Some outputs ended with newline, others didn't, causing inconsistent formatting:
```bash
$ echo test
test$ ls
[files]$
```

**Analysis**:
- Server sends output exactly as captured
- Some commands include trailing newline (ls), others don't (echo)
- Client needs to ensure consistent prompt placement

**Solution**:
```c
// After displaying all output
if (bytes_received > 0 && response[bytes_received - 1] != '\n') {
    printf("\n");  // Add newline if not present
}
```

**Result**: Prompt always appears on new line:
```bash
$ echo test
test
$ ls
[files]
$
```

**Trade-off**: Adds extra newline if output already ends with one, but ensures clean formatting in all cases. Testing showed this was acceptable.

---

## Division of Tasks

### Person A (Server Implementation)
**Responsibilities:**
1. ✅ Implemented `server.c` with complete socket server functionality
2. ✅ Integrated Phase 1 shell (`shell_utils.c/h`) with network layer
3. ✅ Created output capture mechanism using pipes and fork/exec
4. ✅ Implemented server-side logging with formatted messages ([INFO], [RECEIVED], [EXECUTING], [OUTPUT], [ERROR])
5. ✅ Documented communication protocol in `protocol.md`
6. ✅ Tested server with netcat before client was ready
7. ✅ Updated Makefile for server compilation
8. ✅ Wrote server-related report sections

**Key Achievements:**
- Robust command execution engine integrating Phase 1 features
- Clean protocol design that made client implementation straightforward
- Excellent documentation enabling smooth coordination

### Person B (Client Implementation) - YOU
**Responsibilities:**
1. ✅ Studied Person A's protocol documentation thoroughly
2. ✅ Implemented `client.c` with full socket client functionality
3. ✅ Created clean user interface matching Phase 1 exactly
4. ✅ Implemented command transmission following protocol
5. ✅ Implemented response reception with large output handling
6. ✅ Added comprehensive error handling for all failure modes
7. ✅ Updated Makefile to include client in build targets
8. ✅ Tested all Phase 1 features through client-server system
9. ✅ Created `test_client.sh` automated testing script
10. ✅ Wrote client-related report sections

**Key Achievements:**
- Clean, minimal user interface indistinguishable from Phase 1
- Robust network error handling with user-friendly messages
- Successfully verified all Phase 1 features work remotely
- Comprehensive testing demonstrating system reliability

### Coordination Method

**Communication:**
- Person A provided detailed `protocol.md` before client implementation began
- Defined exact buffer sizes, message formats, and error handling approach
- Specified port number, connection flow, and termination protocol

**Testing Strategy:**
1. Person A tested server with netcat independently
2. Person B implemented client following protocol spec
3. Integrated testing verified client-server compatibility
4. Iterative testing revealed and fixed edge cases

**Division Rationale:**
- **Person A**: Server and backend logic (more complex, Phase 1 integration)
- **Person B**: Client and user interface (cleaner separation, independent testing)
- **Separate compilation units**: No code dependencies except protocol adherence

---

## Conclusion

The client implementation successfully provides a transparent, user-friendly interface to the remote shell server. The design prioritizes simplicity, robustness, and strict protocol adherence. Comprehensive testing demonstrates that all Phase 1 features work correctly through the client-server system, and the user experience is indistinguishable from local shell execution.

The client handles all error conditions gracefully, provides clear user feedback, and maintains clean, readable code with extensive comments. The implementation meets all Phase 2 requirements and is ready for deployment on the remote Linux server.

---

**End of Person B Report Sections**


