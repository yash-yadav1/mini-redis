# Mini Redis

A Redis-inspired in-memory key-value store and TCP server built from scratch in **C++17** using POSIX sockets, multithreading, RESP-style protocol parsing, thread synchronization, key expiration (TTL), and file-based persistence.

The project recreates core concepts behind Redis without external networking or database libraries, giving hands-on experience with network programming, concurrent client handling, protocol parsing, in-memory data structures, synchronization, and persistence.

> 📖 See **[commands.md](commands.md)** for copy-pasteable build, run, and test commands.

## Motivation

The goal is to understand how an in-memory database like Redis works internally by implementing its core components from scratch:

- TCP socket creation and connection handling
- Concurrent client processing
- Redis-style RESP request parsing
- Command execution
- Thread-safe shared data access
- Key expiration (TTL) with persisted deadlines
- File-based persistence
- Redis-compatible responses for benchmark workloads

The server was also tested with `redis-benchmark` under concurrent workloads.

## Features

- 🚀 TCP server built directly on POSIX sockets
- 🔄 Concurrent client handling using `std::thread`
- 🔒 Thread-safe database using `std::shared_mutex`
- ⚡ In-memory storage using `std::unordered_map`
- 📡 RESP-style request parsing and RESP-formatted responses
- ⏳ **Key expiration: `EXPIRE`, `TTL`, `PERSIST`**
- 💾 File-based persistence, **including expiration deadlines**
- 🔧 Redis-compatible commands required for benchmark workloads
- 📊 Tested with `redis-benchmark`
- 🔌 Persistent TCP connections with multiple requests per connection
- 📝 Client IP and port logging
- ♻️ Graceful client connection handling

## Tech Stack

| Category | Technology |
|---|---|
| Language | C++17 |
| Networking | POSIX TCP sockets |
| Concurrency | `std::thread` |
| Synchronization | `std::shared_mutex` |
| Data structure | `std::unordered_map` |
| Protocol | RESP-style Redis protocol |
| OS | Linux |
| Persistence | File-based storage |
| Build system | Makefile |
| Testing / benchmarking | `redis-benchmark`, `redis-cli`, `netcat` |

## Supported Commands

| Command | Description |
|---|---|
| `PING` | Check server availability |
| `SET key value` | Store a key-value pair (clears any existing TTL) |
| `GET key` | Retrieve a value |
| `DEL key` | Delete a key |
| `EXISTS key` | Check whether a key exists |
| `SIZE` | Return the number of stored keys |
| `KEYS` | List stored keys (no pattern support) |
| `CLEAR` | Remove all keys |
| `INCR key` | Increment an integer value |
| `EXPIRE key seconds` | Set a time-to-live on a key |
| `TTL key` | Return remaining time-to-live in seconds |
| `PERSIST key` | Remove a key's expiration without deleting the key |
| `QUIT` | Close the client connection |
| `CONFIG GET save` | Return persistence configuration information |
| `CONFIG GET appendonly` | Return AOF compatibility information |

Mini Redis uses `SIZE` and `CLEAR` rather than Redis's `DBSIZE` and `FLUSHDB`. The `CONFIG` implementation is intentionally minimal and exists only for compatibility with Redis client/benchmark behavior.

### TTL return values

| Value | Meaning |
|---|---|
| Positive number | Seconds remaining (rounded down) |
| `-1` | Key exists but has no expiration |
| `-2` | Key does not exist or has expired |

## Key Expiration

- `EXPIRE key seconds` sets an expiration deadline for an existing key.
- `TTL key` reports the remaining time using the values above.
- `PERSIST key` removes the deadline and keeps the key.
- `SET` on an existing key removes its TTL.
- Deadlines are persisted, so the countdown continues across a server restart. A key whose deadline passed while the server was offline is treated as expired after restart.

## RESP Protocol

The server uses a Redis-style RESP (Redis Serialization Protocol) for communication with Redis-compatible clients and benchmarking tools.

A command such as:

```
SET name William
```

is encoded in RESP as:

```
*3\r\n
$3\r\n
SET\r\n
$4\r\n
name\r\n
$7\r\n
William\r\n
```

The server parses the request into tokens (`["SET", "name", "William"]`), the command layer executes it, and the result is serialized back as a RESP response. This separates networking from command execution and database operations.

## System Architecture

```
                         TCP Clients
                             |
                             v
                  +-----------------------+
                  |      TCP Server       |
                  |  socket/bind/listen   |
                  |       accept()        |
                  +-----------+-----------+
                              |
                              v
                  +-----------------------+
                  |     Client Thread     |
                  |      std::thread      |
                  +-----------+-----------+
                              |
                              v
                  +-----------------------+
                  |      RESP Parser      |
                  |      ParseRESP()      |
                  +-----------+-----------+
                              |
                              v
                  +-----------------------+
                  |    Command Executor   |
                  |    executeCommand()   |
                  +-----------+-----------+
                              |
                    +---------+---------+
                    |                   |
                    v                   v
          +------------------+  +-------------------+
          |   In-Memory      |  |   Persistence     |
          |   Database       |  |   Layer           |
          |  unordered_map   |  |  data/database.txt|
          |  + expirations   |  |  (D and E records)|
          +--------+---------+  +-------------------+
                   |
                   v
          +------------------+
          |   shared_mutex   |
          |   Thread Safety  |
          +------------------+
```

## Project Structure

```
MiniRedis/
│
├── include/
│   ├── command.h
│   ├── database.h
│   ├── persistence.h
│   ├── resp.h
│   └── server.h
│
├── src/
│   ├── main.cpp
│   ├── command.cpp
│   ├── database.cpp
│   ├── persistence.cpp
│   ├── resp.cpp
│   └── server.cpp
│
├── data/
│   └── database.txt      (generated at runtime)
│
├── Makefile
├── README.md
├── commands.md
└── .gitignore
```

Networking, command execution, database management, persistence, and protocol parsing are separated into independent modules.

## Build

```bash
make
```

This compiles the C++17 sources with pthread support. To remove build artifacts:

```bash
make clean
```

## Run

```bash
./server
```

The server listens on the default Redis port, `6379`:

```
Server listening on port 6379...
Server ready, Waiting for clients...
```

## Quick Start

In a second terminal:

```bash
redis-cli -p 6379 PING
redis-cli -p 6379 SET session active
redis-cli -p 6379 EXPIRE session 10
redis-cli -p 6379 TTL session
redis-cli -p 6379 GET session
```

For the full set of test scenarios (INCR, EXPIRE/TTL, PERSIST, restart persistence, CLEAR, netcat, benchmarking, troubleshooting), see **[commands.md](commands.md)**.

## Testing with Netcat

```bash
printf '*1\r\n$4\r\nPING\r\n' | nc localhost 6379
```

Expected:

```
+PONG
```

Multiple RESP commands over the same connection:

```bash
printf '*1\r\n$4\r\nPING\r\n*1\r\n$4\r\nPING\r\n' | nc localhost 6379
```

Expected: two `+PONG` responses.

## Benchmarking

```bash
redis-benchmark -p 6379
```

Example workload: 100000 requests, 50 parallel clients, keep-alive enabled. The server handles benchmark workloads including `PING`, `SET`, `GET`, and `INCR`.

`redis-benchmark` may issue commands not implemented here, so treat results as measurements of the supported workload, not a comparison with production Redis.

## Concurrency Model

Each incoming client connection is handled by a separate `std::thread`.

```
                    Server
                       |
              accept connection
                       |
          +------------+------------+
          |            |            |
       Client 1     Client 2     Client 3
          |            |            |
       Thread 1     Thread 2     Thread 3
          |            |            |
          +------------+------------+
                       |
                 Shared Database
```

Because multiple threads access the same `unordered_map`, operations are synchronized with `std::shared_mutex`: reads take a shared lock, writes take an exclusive lock.

## Persistence

State is stored in `data/database.txt`. Records use a simple line-based format:

| Record | Meaning |
|---|---|
| `D` | Key-value data |
| `E` | Expiration deadline for a key |

The persistence layer saves database state to disk and loads it when the server starts. Persisted operations include `SET`, `DEL`, `CLEAR`, `INCR`, `EXPIRE`, and `PERSIST`.

### Known limitation

The file format is whitespace-delimited, so keys or values containing spaces work at runtime but may not be restored correctly after a restart. Escaping/length-prefixing is planned.

## Request Processing Flow

```
Client -> TCP -> Server Socket -> read() -> RESP Parser
       -> Command Tokens -> Command Executor -> Database
       -> RESP Response -> write() -> Client
```

For example, `SET counter 10` becomes `["SET", "counter", "10"]`; the executor updates the in-memory database and returns a RESP success response.

## Design Decisions

**Why POSIX sockets?** Direct experience with `socket()`, `setsockopt()`, `bind()`, `listen()`, `accept()`, `read()`, and `write()`, with no framework hiding them.

**Why `std::thread`?** A thread per client is a simple concurrency model that lets multiple clients interact with the server simultaneously.

**Why `std::unordered_map`?** Redis relies on hash tables; `unordered_map` gives average O(1) lookup, insertion, and deletion.

**Why `std::shared_mutex`?** Allows concurrent reads while writes get exclusive access.

**Why RESP?** It is the protocol Redis clients speak, which makes the server compatible with tools like `redis-cli` and `redis-benchmark`.

**Why store absolute expiration deadlines?** A deadline remains meaningful across restarts, so the TTL countdown continues while the server is offline.

## Learning Outcomes

- TCP/IP networking and POSIX socket programming
- Client-server architecture
- Concurrent programming and thread synchronization
- Hash-table-based data structures
- Protocol parsing and RESP serialization
- Key expiration and time-based state
- File persistence
- Linux systems programming
- Performance benchmarking
- Modular C++ project architecture
- Debugging network protocols

## Future Improvements

- [x] TTL / `EXPIRE` / `PERSIST` with persisted deadlines
- [ ] Safe persistence encoding for keys/values with spaces
- [ ] LRU eviction policy
- [ ] Background persistence thread
- [ ] Multiple logical databases
- [ ] Improved RESP protocol validation
- [ ] Unit and integration tests
- [ ] CMake support
- [ ] Docker support
- [ ] More Redis-compatible commands
- [ ] Improved benchmark and performance analysis

## Author

**Yash Yadav**
Engineering Student @ MNIT Jaipur
Building backend and systems programming projects in C++.