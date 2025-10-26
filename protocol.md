# **File 3: `PROTOCOL.md`** (Documentation for Person B)

# Phase 2 Communication Protocol Documentation
**Author:** Person A  
**Date:** Phase 2 Implementation  
**Purpose:** This document describes the client-server communication protocol for Person B to implement the client

---

## Overview

This document specifies the exact protocol used for communication between the client and server in Phase 2. Person B must follow this protocol precisely to ensure compatibility with the server implementation.

---

## Connection Details

### Server Configuration
- **IP Address:** `localhost` (127.0.0.1) for testing, or any valid IP address
- **Port:** `8080` (hardcoded in server)
- **Protocol:** TCP (connection-oriented, reliable)
- **Connection Type:** One client per server instance (no multi-client support needed)

### Connection Flow
1. Server starts and listens on port 8080
2. Server displays: `[INFO] Server started, waiting for client connections...`
3. Client connects to server
4. Server accepts connection and displays: `[INFO] Client connected.`
5. Client and server enter command/response loop
6. Client sends "exit" to disconnect
7. Connection closes gracefully

---

## Message Format

### Command Messages (Client → Server)

**Format:** Plain text string terminated with newline character

```
<command>\n
```

**Examples:**
```
ls\n
ls -l\n
ps aux\n
cat file.txt\n
ls | grep txt\n
exit\n
```

**Important Notes:**
- Commands are sent as plain ASCII text
- Each command MUST end with a newline character `\n`
- Maximum command length: 4095 characters (BUFFER_SIZE - 1)
- The server will strip the trailing `\n` before processing
- Empty commands (just `\n`) should not be sent - client should skip them

**Sending Logic (Pseudocode):**
```c
// Read command from user
fgets(command, sizeof(command), stdin);

// Send to server (includes the \n from fgets)
send(socket_fd, command, strlen(command), 0);
```

---

### Response Messages (Server → Client)

**Format:** Plain text output (may be multi-line)

```
<output_line_1>\n
<output_line_2>\n
...
<output_line_n>\n
```

**Characteristics:**
- Variable length (can be empty, single line, or many lines)
- No special termination character or delimiter
- Server may send output in multiple chunks for large outputs
- Client must read until `recv()` returns (server closes write after sending complete output)

**Example Outputs:**

For `ls -l`:
```
total 12
-rw-r--r-- 1 user user 1234 Feb 27 12:00 file1.txt
-rw-r--r-- 1 user user 5678 Feb 27 12:05 file2.txt
```

For `unknowncmd`:
```
Error: Command not found: unknowncmd
```

For `exit`:
```
(empty string - no output)
```

**Receiving Logic (Pseudocode):**
```c
char buffer[4096];
ssize_t bytes;

// Read response from server
bytes = recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
if (bytes > 0) {
    buffer[bytes] = '\0';
    printf("%s", buffer);  // Display to user
}
```

**Important:** For large outputs, you may need to call `recv()` multiple times in a loop until all data is received.

---

## Communication Protocol Flow

### Normal Command Execution

```
Client                          Server
  |                               |
  |--- "ls -l\n" ---------------→ | [RECEIVED] Received command: "ls -l" from client.
  |                               | [EXECUTING] Executing command: "ls -l"
  |                               | [OUTPUT] Sending output to client:
  |                               | <displays output on server console>
  |←-- "total 12\n-rw-r--..." ---| 
  |                               |
(displays output)                 |
```

### Error Case (Command Not Found)

```
Client                          Server
  |                               |
  |--- "badcmd\n" --------------→ | [RECEIVED] Received command: "badcmd" from client.
  |                               | [EXECUTING] Executing command: "badcmd"
  |                               | [ERROR] Command not found: "badcmd"
  |                               | [OUTPUT] Sending error message to client:
  |←-- "Error: Command..." ------| 
  |                               |
(displays error)                  |
```

### Exit Command

```
Client                          Server
  |                               |
  |--- "exit\n" ----------------→ | [RECEIVED] Received command: "exit" from client.
  |                               | [INFO] Client requested exit.
  |←-- "" (empty) ----------------| 
  |                               |
(closes connection)         (closes connection)
```

---

## Buffer Sizes

### Defined Constants
```c
#define BUFFER_SIZE 4096
```

- **Command Buffer:** Client should use 4096 bytes for receiving user input
- **Response Buffer:** Client should use 4096 bytes for receiving server output
- **Note:** Large outputs may require multiple `recv()` calls

---

## Error Handling

### Client-Side Errors to Handle

1. **Connection Refused**
   - Occurs when: Server is not running
   - Action: Display error message "Error: Cannot connect to server at <IP>:<PORT>"
   - Example: `Error: Cannot connect to server at localhost:8080`

2. **Connection Lost During Operation**
   - Occurs when: Server crashes or network failure
   - Detection: `recv()` returns 0 or -1
   - Action: Display "Error: Connection to server lost" and exit

3. **Send Failure**
   - Occurs when: Network error during send
   - Detection: `send()` returns -1
   - Action: Display error and attempt to close gracefully

4. **Receive Failure**
   - Occurs when: Network error during receive
   - Detection: `recv()` returns -1 (and errno != EAGAIN)
   - Action: Display error and close connection

---

## Special Cases

### Empty Command
- **User Input:** Just presses Enter (empty line)
- **Client Action:** Should NOT send to server, just display prompt again
- **Reason:** Avoid unnecessary network traffic

### Very Long Commands
- **Maximum Length:** 4095 characters (BUFFER_SIZE - 1)
- **Client Action:** If user enters command longer than buffer, truncate or reject
- **Note:** Phase 1 commands shouldn't normally exceed this length

### Commands with Multiple Lines of Output
- **Example:** `ls -la` in a directory with many files
- **Client Action:** May need to call `recv()` in a loop to get all output
- **Recommendation:** Read until `recv()` returns 0 or less

### Commands with No Output
- **Example:** `ls nonexistent 2> /dev/null`
- **Server Sends:** Empty string
- **Client Action:** Just display prompt again (no output to show)

---

## Client Implementation Checklist

Person B should implement the following:

### Socket Setup
- [ ] Create TCP socket using `socket(AF_INET, SOCK_STREAM, 0)`
- [ ] Configure server address structure with IP and port 8080
- [ ] Connect to server using `connect()`
- [ ] Handle connection failures gracefully

### User Interface
- [ ] Display `$ ` prompt (exactly like Phase 1)
- [ ] Read user input using `fgets()` or similar
- [ ] Trim/validate input
- [ ] Skip empty commands (don't send to server)

### Command Transmission
- [ ] Send commands with `send()`, including newline
- [ ] Handle send errors
- [ ] Special handling for "exit" command

### Response Reception
- [ ] Receive output using `recv()`
- [ ] Handle multi-line output (loop if needed)
- [ ] Display output to user exactly as received
- [ ] Handle receive errors

### Error Handling
- [ ] Connection refused errors
- [ ] Connection lost during operation
- [ ] Send/receive failures
- [ ] User-friendly error messages

### Clean Exit
- [ ] Send "exit" command to server
- [ ] Close socket properly
- [ ] Handle Ctrl+D (EOF) gracefully

---

## Testing Recommendations

### Test with Server Running
1. Start server: `./server`
2. In another terminal, start client: `./client`
3. Test commands:
   - Simple: `ls`, `pwd`, `echo hello`
   - Arguments: `ls -l`, `ps aux`
   - Pipes: `ls | grep txt`
   - Redirections: `cat < input.txt`
   - Errors: `unknowncmd`
   - Exit: `exit`

### Test Error Cases
1. Start client without server running (should show connection error)
2. Kill server while client is connected (should detect disconnection)
3. Try very long commands
4. Try empty input (should not send to server)

### Test with netcat (Before Client is Ready)
```bash
# Terminal 1: Start server
./server

# Terminal 2: Connect with netcat
nc localhost 8080

# Type commands:
ls
pwd
exit
```

---

## Example Client Pseudocode

```c
int main() {
    // 1. Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    // 2. Configure server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
    
    // 3. Connect
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error: Cannot connect to server\n");
        return 1;
    }
    
    // 4. Main loop
    char command[4096];
    char response[4096];
    
    while (1) {
        // Display prompt
        printf("$ ");
        fflush(stdout);
        
        // Read command
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;  // EOF
        }
        
        // Skip empty commands
        if (command[0] == '\n') continue;
        
        // Send command to server
        send(sock, command, strlen(command), 0);
        
        // Check for exit
        if (strncmp(command, "exit", 4) == 0) {
            break;
        }
        
        // Receive and display response
        ssize_t bytes = recv(sock, response, sizeof(response) - 1, 0);
        if (bytes > 0) {
            response[bytes] = '\0';
            printf("%s", response);
        }
    }
    
    // 5. Cleanup
    close(sock);
    return 0;
}
```

---

## Contact

If you have any questions about the protocol or encounter issues during integration:
- Review this document thoroughly
- Check server.c implementation for reference
- Test with `nc` or `telnet` first
- Verify buffer sizes and message formats

---

## Protocol Summary (Quick Reference)

| Aspect | Specification |
|--------|--------------|
| **Port** | 8080 |
| **Protocol** | TCP |
| **Command Format** | `<command>\n` |
| **Response Format** | Plain text (variable length) |
| **Buffer Size** | 4096 bytes |
| **Exit Command** | `"exit\n"` |
| **Connection Type** | Single client per server |
| **Message Termination** | Newline for commands, none for responses |

---

**END OF PROTOCOL DOCUMENTATION**