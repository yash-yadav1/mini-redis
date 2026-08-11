# Mini Redis

A Redis-inspired **in-memory key-value store and TCP server** built from scratch in **C++17** using **POSIX sockets, multithreading, RESP-style protocol parsing, thread synchronization, and file-based persistence**.

The project recreates core concepts behind Redis without using external networking or database libraries, providing hands-on experience with **network programming, concurrent client handling, protocol parsing, in-memory data structures, synchronization, and persistence**.

---

## Motivation

The goal of this project is to understand how an in-memory database such as Redis works internally by implementing its core components from scratch.

Instead of relying on networking frameworks or database libraries, the project manually implements:

* TCP socket creation and connection handling
* Concurrent client processing
* Redis-style RESP request parsing
* Command execution
* Thread-safe shared data access
* File-based persistence
* Redis-compatible responses for benchmark workloads

The project was also tested using **`redis-benchmark`** to evaluate the server under concurrent workloads.

---

## Features

* 🚀 TCP server built directly using POSIX sockets
* 🔄 Concurrent client handling using `std::thread`
* 🔒 Thread-safe database using `std::shared_mutex`
* ⚡ In-memory storage using `std::unordered_map`
* 📡 RESP-style request parsing
* 📤 RESP-formatted server responses
* 💾 File-based persistence
* 🔧 Redis-compatible commands required for benchmark workloads
* 📊 Tested with `redis-benchmark`
* 🔌 Persistent TCP connections with multiple requests per connection
* 📝 Client IP and port logging
* ♻️ Graceful client connection handling

---

## Tech Stack

| Category               | Technology                  |
| ----------------------- | ---------------------------- |
| Language               | C++17                       |
| Networking             | POSIX TCP Sockets           |
| Concurrency            | `std::thread`               |
| Synchronization        | `std::shared_mutex`         |
| Data Structure         | `std::unordered_map`        |
| Protocol               | RESP-style Redis protocol   |
| Operating System       | Linux                       |
| Persistence            | File-based storage          |
| Build System           | Makefile                    |
| Testing / Benchmarking | `redis-benchmark`, `netcat` |

---

## Supported Commands

| Command                 | Description                                  |
| ------------------------ | --------------------------------------------- |
| `PING`                  | Check server availability                    |
| `SET key value`         | Store a key-value pair                       |
| `GET key`               | Retrieve a value                             |
| `DEL key`               | Delete a key                                 |
| `EXISTS key`            | Check whether a key exists                   |
| `SIZE`                  | Return the number of stored keys             |
| `KEYS`                  | List stored keys                             |
| `CLEAR`                 | Remove all keys                              |
| `INCR key`              | Increment an integer value                   |
| `QUIT`                  | Close the client connection                  |
| `CONFIG GET save`       | Return persistence configuration information |
| `CONFIG GET appendonly` | Return AOF compatibility information         |

The `CONFIG` implementation is intentionally minimal and exists primarily to provide compatibility with Redis client/benchmark behavior rather than reproduce Redis's complete configuration subsystem.

---

## RESP Protocol

The server uses a Redis-style **RESP (Redis Serialization Protocol)** for communication with Redis-compatible clients and benchmarking tools.

For example, a command such as:

```text
SET name William
```

is represented using RESP as:

```text
*3\r\n
$3\r\n
SET\r\n
$4\r\n
name\r\n
$6\r\n
William\r\n
```

The server parses the incoming request into command tokens:

```text
["SET", "name", "William"]
```

The command is then executed by the command layer and the result is serialized back into a RESP-compatible response.

This separates the networking layer from command execution and database operations.

---

## System Architecture

```text
                         TCP Clients
                             |
                             v
                  +-----------------------+
                  |     TCP Server        |
                  | socket/bind/listen    |
                  |       accept()        |
                  +-----------+-----------+
                              |
                              v
                  +-----------------------+
                  |    Client Thread      |
                  |     std::thread       |
                  +-----------+-----------+
                              |
                              v
                  +-----------------------+
                  |     RESP Parser       |
                  |      ParseRESP()      |
                  +-----------+-----------+
                              |
                              v
                  +-----------------------+
                  |   Command Executor    |
                  |   executeCommand()    |
                  +-----------+-----------+
                              |
                    +---------+---------+
                    |                   |
                    v                   v
          +------------------+  +-------------------+
          | In-Memory        |  | Persistence       |
          | Database         |  | Layer             |
          | unordered_map    |  | database.txt      |
          +--------+---------+  +-------------------+
                   |
                   v
          +------------------+
          | shared_mutex     |
          | Thread Safety    |
          +------------------+
```

---

## Project Structure

```text
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
├── Makefile
├── README.md
└── .gitignore
```

The project is separated into modules so that networking, command execution, database management, persistence, and protocol parsing remain independent.

---

## Build

The project uses a `Makefile` for compilation.

Build the server with:

```bash
make
```

This compiles the C++17 source files with pthread support.

To remove generated build artifacts:

```bash
make clean
```

---

## Run

Start the server:

```bash
./server
```

The server listens on the default Redis port:

```text
6379
```

Expected output:

```text
Server listening on port 6379...
Server ready, Waiting for clients...
```

---

## Testing with Netcat

A raw TCP client can be used for basic connectivity testing:

```bash
nc localhost 6379
```

For RESP commands, requests can also be sent directly through `nc`.

For example:

```bash
printf '*1\r\n$4\r\nPING\r\n' | nc localhost 6379
```

Expected response:

```text
+PONG
```

Multiple RESP commands can be sent over the same TCP connection:

```bash
printf '*1\r\n$4\r\nPING\r\n*1\r\n$4\r\nPING\r\n' | nc localhost 6379
```

Expected:

```text
+PONG
+PONG
```

---

## Benchmarking

The server has been tested using Redis's `redis-benchmark` utility.

Example:

```bash
redis-benchmark -p 6379
```

The benchmark sends multiple requests concurrently and measures the server's throughput and latency.

Example workload:

```text
100000 requests
50 parallel clients
keep alive: 1
```

The server successfully handles benchmark workloads including:

* `PING`
* `SET`
* `GET`
* `INCR`

This demonstrates that the server can process large numbers of requests across multiple concurrent TCP clients.

---

## Concurrency Model

Each incoming client connection is handled by a separate `std::thread`.

Conceptually:

```text
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

Because multiple threads access the same `unordered_map`, database operations are synchronized using `std::shared_mutex`.

Read operations can use shared locking, while write operations require exclusive access.

This prevents concurrent clients from corrupting shared database state.

---

## Persistence

The database state is stored in:

```text
database.txt
```

The persistence layer provides functionality to:

* Save database state to disk
* Load database state when the server starts

Persistent operations include:

* `SET`
* `DEL`
* `CLEAR`
* `INCR`

This allows data to survive a server restart.

---

## Request Processing Flow

A client request follows this path:

```text
Client
  |
  | TCP
  v
Server Socket
  |
  v
read()
  |
  v
RESP Parser
  |
  v
Command Tokens
  |
  v
Command Executor
  |
  v
Database
  |
  v
RESP Response
  |
  v
write()
  |
  v
Client
```

For example:

```text
SET counter 10
```

becomes:

```text
["SET", "counter", "10"]
```

The command executor then updates the in-memory database and returns a RESP-compatible success response.

---

## Design Decisions

### Why POSIX sockets?

Using raw POSIX sockets provides direct experience with the fundamentals of TCP networking:

* `socket()`
* `setsockopt()`
* `bind()`
* `listen()`
* `accept()`
* `read()`
* `write()`

No networking framework hides these operations.

### Why `std::thread`?

A dedicated thread for each client provides a simple concurrency model and allows multiple clients to interact with the server simultaneously.

### Why `std::unordered_map`?

Redis relies heavily on hash-table-based data structures for efficient key-value access.

`std::unordered_map` provides average **O(1)** lookup, insertion, and deletion.

### Why `std::shared_mutex`?

Multiple clients may access the database simultaneously.

Shared locking allows concurrent reads while exclusive locking protects modifications.

### Why RESP?

RESP is the protocol used by Redis clients to communicate with Redis servers. Implementing RESP-style parsing makes the project more realistic and allows compatibility with Redis tooling such as `redis-benchmark`.

---

## Learning Outcomes

This project provided hands-on experience with:

* TCP/IP networking
* POSIX socket programming
* Client-server architecture
* Concurrent programming
* Thread synchronization
* `std::thread`
* `std::shared_mutex`
* Hash-table based data structures
* Protocol parsing
* RESP serialization
* File persistence
* Linux systems programming
* Performance benchmarking
* Modular C++ project architecture
* Debugging network protocols

---

## Future Improvements

* [ ] TTL / `EXPIRE` command
* [ ] LRU eviction policy
* [ ] Background persistence thread
* [ ] Multiple logical databases
* [ ] Improved RESP protocol validation
* [ ] Unit and integration tests
* [ ] CMake support
* [ ] Docker support
* [ ] More Redis-compatible commands
* [ ] Improved benchmark and performance analysis

---

## Author

**Yash Yadav**

Engineering Student @ MNIT Jaipur

Building backend and systems programming projects in C++.