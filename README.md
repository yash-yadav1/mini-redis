# Mini Redis

A Redis-inspired **in-memory key-value store** built from scratch in **C++17** using **POSIX sockets**, **multithreading**, and **file-based persistence**.

This project demonstrates the core concepts behind an in-memory database by implementing networking, concurrent client handling, command parsing, thread synchronization, and persistent storage without relying on external libraries.

---

## Motivation

The goal of this project is to understand how databases like **Redis** work internally by implementing their core components from scratch.

Instead of using existing networking frameworks or database libraries, every major component—including the TCP server, command parser, client handling, and persistence—is written manually in C++.

---

## Features

- 🚀 TCP server built directly using POSIX sockets
- 🔄 Concurrent client handling using `std::thread`
- 🔒 Thread-safe shared database protected with `std::mutex`
- ⚡ In-memory key-value store using `std::unordered_map`
- 💾 Automatic persistence to disk (`database.txt`)
- 📡 Simple line-based text protocol
- 🔧 Compatible with `netcat` and `telnet`

---

## Tech Stack

| Category | Technology |
|----------|------------|
| Language | C++17 |
| Networking | POSIX Sockets |
| Concurrency | `std::thread`, `std::mutex` |
| Data Structure | `std::unordered_map` |
| Operating System | Linux |
| Persistence | Flat-file (`database.txt`) |

---

## Supported Commands

| Command | Description |
|---------|-------------|
| `PING` | Check server availability |
| `SET key value` | Store a key-value pair |
| `GET key` | Retrieve a value |
| `DEL key` | Delete a key |
| `EXISTS key` | Check if a key exists |
| `SIZE` | Return total number of keys |
| `KEYS` | List all stored keys |
| `CLEAR` | Remove all keys |
| `INCR key` | Increment an integer value |
| `QUIT` | Close client connection |

---

## System Architecture

```text
                 +--------------------+
                 |    TCP Clients     |
                 +----------+---------+
                            |
                            v
                  +-------------------+
                  |   POSIX Socket    |
                  | (listen/accept)   |
                  +---------+---------+
                            |
                            v
                  +-------------------+
                  | Client Thread     |
                  | (std::thread)     |
                  +---------+---------+
                            |
                            v
                  +-------------------+
                  | Command Parser    |
                  +---------+---------+
                            |
             +--------------+--------------+
             |                             |
             v                             v
+---------------------------+   +---------------------------+
| In-Memory Database        |   | Persistent Storage        |
| unordered_map<string,...> |   | database.txt             |
+---------------------------+   +---------------------------+
```

---

## Build

Compile using:

```bash
g++ -std=c++17 -pthread server.cpp -o server
```

---

## Run

Start the server:

```bash
./server
```

Expected output:

```text
Server listening on port 6379...
Server ready, Waiting for clients...
```

---

## Example

Connect using:

```bash
nc localhost 6379
```

Then execute:

```text
PING
PONG

SET name William
OK

GET name
William

INCR counter
1

INCR counter
ok new val: 2

EXISTS name
true

KEYS
name
counter

SIZE
2

DEL name
OK

QUIT
bye!
```

---

## Persistence

Every write operation automatically updates `database.txt`.

Supported persistent operations:

- `SET`
- `DEL`
- `CLEAR`
- `INCR`

The database is automatically loaded when the server starts.

---

## Project Structure

```text
mini-redis/
│
├── server.cpp
├── README.md
├── .gitignore
├── database.txt
└── server
```

---

## How It Works

1. Create a TCP socket.
2. Bind to port **6379**.
3. Listen for incoming client connections.
4. Spawn a new thread for every client.
5. Parse client commands.
6. Execute commands on the shared in-memory database.
7. Protect shared data using a mutex.
8. Persist changes to disk.
9. Send responses back to the client.

---

## Roadmap

### ✅ Completed

- [x] TCP server using POSIX sockets
- [x] Multi-threaded client handling
- [x] Thread-safe database
- [x] File persistence
- [x] Redis-inspired command set
- [x] Client connection logging

### 🚧 Planned

- [ ] RESP protocol support
- [ ] TTL / EXPIRE command
- [ ] LRU eviction policy
- [ ] Background persistence thread
- [ ] Multiple logical databases
- [ ] Benchmarking tool
- [ ] Unit tests
- [ ] CMake support
- [ ] Docker support

---

## Learning Outcomes

This project provided hands-on experience with:

- TCP/IP Networking
- POSIX Socket Programming
- Concurrent Programming
- Thread Synchronization
- Linux Systems Programming
- In-memory Data Structures
- File Persistence
- Designing a Redis-inspired Server Architecture

---

## Author

**Yash Yadav**

Engineering Student @ MNIT Jaipur

Currently building backend and systems programming projects in C++.