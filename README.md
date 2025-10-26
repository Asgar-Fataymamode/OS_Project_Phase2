# **File 5: `README.md`** (Project Overview and Quick Start)

```markdown
# Remote Shell - Phase 2
**Operating Systems Lab Project**  
**NYU Abu Dhabi**

---

## Table of Contents
1. [Project Overview](#project-overview)
2. [Phase 2 Architecture](#phase-2-architecture)
3. [Features](#features)
4. [File Structure](#file-structure)
5. [Quick Start](#quick-start)
6. [Compilation](#compilation)
7. [Usage](#usage)
8. [Protocol Specification](#protocol-specification)
9. [Team Division](#team-division)
10. [Testing](#testing)
11. [Troubleshooting](#troubleshooting)
12. [Known Limitations](#known-limitations)
13. [Future Enhancements](#future-enhancements)

---

## Project Overview

This project implements a **remote shell system** using TCP socket communication, extending the Phase 1 local shell into a distributed client-server architecture. Users can connect to the server remotely and execute shell commands as if they were on the local machine.

### What This Does
- **Server**: Listens for client connections on port 8080
- **Client**: Connects to server, provides shell prompt, sends commands
- **Execution**: Server executes commands using Phase 1 shell engine
- **Response**: Server captures output and sends it back to client
- **Display**: Client displays output exactly like a local shell

### Key Technologies
- **Language**: C (C99 standard)
- **Networking**: TCP sockets (IPv4)
- **IPC**: Pipes for output capture
- **Process Management**: fork(), exec(), wait()
- **Phase 1 Integration**: Complete reuse of shell parsing and execution

---

## Phase 2 Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                     REMOTE SHELL SYSTEM                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│    LOCAL MACHINE                          REMOTE SERVER          │
│    ┌─────────────┐                       ┌─────────────┐        │
│    │   CLIENT    │                       │   SERVER    │        │
│    │  (PersonB)  │                       │  (PersonA)  │        │
│    │             │    TCP Connection     │             │        │
│    │  ┌───────┐  │◄─────(Port 8080)────►│  ┌───────┐  │        │
│    │  │ User  │  │                       │  │Socket │  │        │
│    │  │  I/O  │  │   1. Send Command     │  │Handler│  │        │
│    │  └───┬───┘  │  ─────────────────►   │  └───┬───┘  │        │
│    │      │      │                       │      │      │        │
│    │      │      │   2. Execute via      │      ▼      │        │
│    │  ┌───▼───┐  │      Phase 1          │  ┌────────┐ │        │
│    │  │Display│  │                       │  │ Phase1 │ │        │
│    │  │Output │  │◄─ 3. Return Output ── │  │ Shell  │ │        │
│    │  └───────┘  │                       │  │ Engine │ │        │
│    │      ▲      │                       │  └────────┘ │        │
│    │      │      │                       │      │      │        │
│    │  $ ls -l    │                       │  Captures   │        │
│    │  total 12   │                       │  stdout/    │        │
│    │  -rw-r--... │                       │  stderr     │        │
│    └─────────────┘                       └─────────────┘        │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘

Flow:
1. User types command in client → Client sends to server
2. Server receives → Executes using Phase 1 code → Captures output
3. Server sends output back → Client displays to user
```

---

## Features

### Phase 1 Features (Fully Preserved)
✅ Simple commands (`ls`, `pwd`, `echo`)  
✅ Commands with arguments (`ls -l`, `ps aux`)  
✅ Input redirection (`sort < input.txt`)  
✅ Output redirection (`ls > output.txt`)  
✅ Error redirection (`command 2> error.log`)  
✅ Pipes (`ls | grep txt`)  
✅ Multiple pipes (`cmd1 | cmd2 | cmd3`)  
✅ Compound commands (`cmd < in.txt | cmd2 > out.txt`)  
✅ Built-in commands (`echo` with `-e` flag)  
✅ Error handling (invalid commands, missing files)  

### Phase 2 Features (New)
✅ Remote command execution via TCP sockets  
✅ Client-server architecture  
✅ Output capture and transmission  
✅ Server-side logging with formatted display  
✅ Protocol for reliable communication  
✅ Graceful connection handling  
✅ Support for arbitrary output sizes (dynamic buffering)  
✅ Clean disconnection on `exit`  

---

## File Structure

```
project/
│
├── Phase 1 Files (Reused)
│   ├── myshell.c              # Original local shell (standalone)
│   ├── shell_utils.c          # Shell parsing & execution engine
│   └── shell_utils.h          # Shell data structures & prototypes
│
├── Phase 2 Files (New)
│   ├── server.c               # Server implementation (Person A)
│   ├── client.c               # Client implementation (Person B)
│   ├── PROTOCOL.md            # Communication protocol documentation
│   └── REPORT_PERSON_A.md     # Person A's report sections
│
├── Build System
│   └── Makefile               # Updated for Phase 2 targets
│
└── Documentation
    └── README.md              # This file
```

### File Descriptions

| File | Lines | Purpose | Owner |
|------|-------|---------|-------|
| `server.c` | ~580 | Server socket management, command execution, output capture | Person A |
| `client.c` | ~300 | Client socket connection, user I/O, command transmission | Person B |
| `shell_utils.c` | ~800 | Phase 1 parsing and execution logic | Phase 1 (reused) |
| `shell_utils.h` | ~90 | Phase 1 data structures and prototypes | Phase 1 (reused) |
| `myshell.c` | ~70 | Phase 1 local shell main function | Phase 1 (kept for reference) |
| `Makefile` | ~120 | Build system for all targets | Both |
| `PROTOCOL.md` | ~500 | Protocol specification for Person B | Person A |

---

## Quick Start

### Prerequisites
- Linux environment (Ubuntu, Debian, etc.)
- gcc compiler
- make utility
- Network access (localhost testing)

### 5-Minute Setup

```bash
# 1. Clone or extract project files
cd /path/to/project

# 2. Compile server
make server

# 3. Start server (Terminal 1)
./server

# 4. Test with netcat (Terminal 2)
nc localhost 8080

# 5. Type commands
ls
pwd
exit

# Success! Server is working ✓
```

### With Client (After Person B Completes)

```bash
# Terminal 1: Start server
./server

# Terminal 2: Start client
./client

# Client will show prompt:
$ _
```

---

## Compilation

### Build Commands

```bash
# Build everything (server + Phase 1 shell)
make all

# Build only server
make server

# Build only client (after Person B completes)
make client

# Build only Phase 1 shell
make myshell

# Clean all generated files
make clean

# Clean and rebuild
make rebuild
```

### Compilation Output

Successful compilation:
```
$ make server
Compiling server.c...
Compiling shell_utils.c...
Linking server...
Build successful: server
```

### Compilation Flags

The Makefile uses these compiler flags:
- `-Wall` - Enable all warnings
- `-Wextra` - Enable extra warnings
- `-std=c99` - Use C99 standard
- `-g` - Include debugging symbols
- `-pedantic` - Strict ISO C compliance

---

## Usage

### Server Usage

#### Start Server
```bash
./server
```

**Output:**
```
[INFO] Server started, waiting for client connections...
```

Server is now listening on port 8080 for incoming connections.

#### Server Output Format

When client connects and executes commands:
```
[INFO] Client connected.
[RECEIVED] Received command: "ls -l" from client.
[EXECUTING] Executing command: "ls -l"
[OUTPUT] Sending output to client:
total 12
-rw-r--r-- 1 user user 1234 Feb 27 12:00 file1.txt
-rw-r--r-- 1 user user 5678 Feb 27 12:05 file2.txt
```

For errors:
```
[RECEIVED] Received command: "unknowncmd" from client.
[EXECUTING] Executing command: "unknowncmd"
[ERROR] Command not found: "unknowncmd"
[OUTPUT] Sending error message to client:
Error: Command not found: unknowncmd
```

#### Stop Server
- Wait for client to send `exit`
- Or press `Ctrl+C` to force stop

---

### Client Usage (After Person B Completes)

#### Start Client
```bash
./client
```

**Output:**
```
$ _
```

#### Using Client

The client provides a shell prompt identical to Phase 1:

```bash
$ ls
Makefile  myshell  server  shell_utils.c  shell_utils.h

$ ls -l
total 64
-rw-r--r-- 1 user user  2377 Oct 26 Makefile
-rwxr-xr-x 1 user user 18456 Oct 26 myshell
-rwxr-xr-x 1 user user 19872 Oct 26 server

$ ps | grep bash
1234 pts/0    00:00:00 bash

$ exit
```

Client exits and disconnects from server.

---

### Testing with netcat (Before Client is Ready)

Netcat allows testing the server without the client:

```bash
# Terminal 1: Start server
./server

# Terminal 2: Connect with netcat
nc localhost 8080

# Type commands directly:
ls
pwd
whoami
echo hello
exit
```

---

## Protocol Specification

### Quick Reference

| Aspect | Value |
|--------|-------|
| **Protocol** | TCP |
| **Port** | 8080 |
| **Command Format** | `<command>\n` |
| **Response Format** | Plain text (variable length) |
| **Buffer Size** | 4096 bytes |
| **Exit Command** | `exit` |

### Message Flow

#### Normal Command
```
Client                          Server
  |                               |
  |--- "ls\n" -------------------→| 
  |                               | [Execute]
  |←-- "file1.txt\nfile2.txt\n" -|
  |                               |
```

#### Error Command
```
Client                          Server
  |                               |
  |--- "badcmd\n" ---------------→| 
  |                               | [Error]
  |←-- "Error: Command not..." --|
  |                               |
```

#### Exit
```
Client                          Server
  |                               |
  |--- "exit\n" -----------------→| 
  |←-- "" (empty) ----------------|
  |                               |
[Disconnect]              [Close connection]
```

For complete protocol documentation, see `PROTOCOL.md`.

---

## Team Division

### Person A (Server Implementation)
**Files:**
- `server.c`
- `PROTOCOL.md`
- `REPORT_PERSON_A.md`
- Updated `Makefile`

**Responsibilities:**
- Socket setup and management
- Connection handling
- Command execution with output capture
- Phase 1 integration
- Server-side logging
- Protocol design and documentation

**Status:** ✅ Complete

---

### Person B (Client Implementation)
**Files:**
- `client.c`
- Complete report assembly

**Responsibilities:**
- Socket connection to server
- User interface (prompt, input)
- Command transmission
- Output reception and display
- Error handling
- Final Makefile updates

**Status:** 🔄 In Progress

---

## Testing

### Manual Testing (Without Client)

#### Test 1: Basic Connection
```bash
# Terminal 1
./server

# Terminal 2
nc localhost 8080
# Type: ls
# Type: exit
```

#### Test 2: Phase 1 Features
```bash
nc localhost 8080
ls -l
ps aux | grep bash
echo test > output.txt
cat < output.txt
exit
```

#### Test 3: Error Handling
```bash
nc localhost 8080
unknowncmd
invalidcommand123
exit
```

### Automated Testing (Future)

```bash
# Run test suite (to be implemented)
make test-server
```

### Integration Testing (With Client)

```bash
# Terminal 1: Server
./server

# Terminal 2: Client
./client
# Execute various commands
# Verify output matches expectations
```

---

## Troubleshooting

### Problem: "Address already in use"

**Cause:** Server still running or port not released

**Solution:**
```bash
# Find process using port 8080
lsof -i :8080

# Kill the process
kill -9 <PID>

# Or wait 1-2 minutes for port to be released
```

---

### Problem: "Connection refused"

**Cause:** Server not running or wrong port

**Solution:**
```bash
# Verify server is running
ps aux | grep server

# Check server is listening on 8080
netstat -tuln | grep 8080

# Restart server
./server
```

---

### Problem: Client shows garbled output

**Cause:** Protocol mismatch or buffer issues

**Solution:**
- Verify client follows `PROTOCOL.md` exactly
- Check buffer sizes (4096 bytes)
- Ensure proper null-termination of strings

---

### Problem: Server crashes on large output

**Cause:** Memory allocation failure or buffer overflow

**Solution:**
- Check available memory: `free -h`
- Verify dynamic buffer reallocation in `execute_command_with_capture()`
- Test with: `ls -la /usr/bin`

---

### Problem: Zombie processes accumulating

**Cause:** Server not calling `waitpid()` for child processes

**Solution:**
```bash
# Check for zombies
ps aux | grep defunct

# Server should call waitpid() - verify in code
# Restart server if zombies present
```

---

## Known Limitations

### Current Limitations

1. **Single Client Only**
   - Server handles one client at a time
   - Additional clients must wait for current client to disconnect
   - *Future:* Add multi-threading for concurrent clients

2. **No Authentication**
   - No username/password required
   - Anyone who can connect can execute commands
   - *Future:* Add authentication mechanism

3. **Localhost Only (Default)**
   - Server binds to all interfaces but typically used on localhost
   - No encryption (plain text transmission)
   - *Future:* Add TLS/SSL for secure remote connections

4. **Limited Error Recovery**
   - Server exits on critical errors (bind failure, etc.)
   - *Future:* Add automatic restart or error recovery

5. **No Command History**
   - Client doesn't maintain command history
   - *Future:* Add readline support for history and autocomplete

6. **No Session Persistence**
   - Each connection is independent
   - Working directory resets on reconnect
   - *Future:* Add session management

### Phase 1 Limitations (Inherited)

All Phase 1 limitations still apply:
- Maximum command length: 4095 characters
- Maximum arguments: 63 per command
- No background processes (`&` operator)
- Limited built-in commands (only `echo`)

### Testing Commands
```bash
# Memory leaks
valgrind --leak-check=full ./server

# File descriptors
lsof -p $(pidof server)

# Network connections
netstat -tuln | grep 8080
ss -tuln | grep 8080

# Process tree
pstree -p $(pidof server)
```

## Quick Command Reference

```bash
# Compilation
make server          # Build server
make client          # Build client
make all            # Build everything
make clean          # Remove generated files

# Execution
./server            # Start server
./client            # Start client (after implementation)
nc localhost 8080   # Test with netcat

# Testing
make test-server    # Run server tests
lsof -i :8080      # Check port usage
ps aux | grep server # Check server process

# Debugging
gdb ./server        # Debug server
valgrind ./server   # Check memory leaks
```