# Mini Redis — Commands Guide

Copy-pasteable commands for building, running, and testing Mini Redis.

> Mini Redis implements a subset of Redis. See the [command reference](#command-reference) below. Use `SIZE` instead of `DBSIZE` and `CLEAR` instead of `FLUSHDB`.

## Table of Contents

1. [Build](#1-build)
2. [Start the server](#2-start-the-server)
3. [Send commands with redis-cli](#3-send-commands-with-redis-cli)
4. [Run multiple commands in one batch](#4-run-multiple-commands-in-one-batch)
5. [Values containing spaces](#5-values-containing-spaces)
6. [INCR](#6-incr)
7. [EXPIRE and TTL](#7-expire-and-ttl)
8. [PERSIST](#8-persist)
9. [Expiration across a server restart](#9-expiration-across-a-server-restart)
10. [DEL and CLEAR](#10-del-and-clear)
11. [CONFIG responses](#11-config-responses)
12. [Testing with netcat](#12-testing-with-netcat)
13. [Benchmarking](#13-benchmarking)
14. [Inspect persisted data](#14-inspect-persisted-data)
15. [Troubleshooting](#15-troubleshooting)
16. [Stop and rebuild](#16-stop-and-rebuild)

---

## Command Reference

| Command | Description |
|---|---|
| `PING` | Check server availability |
| `SET key value` | Store a key-value pair (clears any existing TTL on the key) |
| `GET key` | Retrieve a value |
| `DEL key` | Delete a key |
| `EXISTS key` | Check whether a key exists |
| `SIZE` | Number of stored keys |
| `KEYS` | List stored keys (no pattern support) |
| `CLEAR` | Remove all keys |
| `INCR key` | Increment an integer value |
| `EXPIRE key seconds` | Set a time-to-live on a key |
| `TTL key` | Remaining time-to-live in seconds |
| `PERSIST key` | Remove the expiration but keep the key |
| `CONFIG GET save` | Compatibility response |
| `CONFIG GET appendonly` | Compatibility response |
| `QUIT` | Close the client connection |

---

## 1. Build

From the project root:

```bash
cd ~/MiniRedis
make
```

Clean and rebuild:

```bash
make clean
make
```

## 2. Start the server

**Terminal 1:**

```bash
cd ~/MiniRedis
./server
```

Leave this terminal running. The server listens on port `6379` by default.

If the port is already in use, check whether another Mini Redis or Redis server is running before starting a new instance.

## 3. Send commands with redis-cli

**Terminal 2:**

```bash
redis-cli -p 6379 PING
redis-cli -p 6379 SET name yash
redis-cli -p 6379 GET name
redis-cli -p 6379 EXISTS name
redis-cli -p 6379 SIZE
redis-cli -p 6379 KEYS
redis-cli -p 6379 DEL name
redis-cli -p 6379 GET name
```

## 4. Run multiple commands in one batch

Paste the whole block into Terminal 2:

```bash
redis-cli -p 6379 <<'EOF'
PING
SET name yash
GET name
EXISTS name
SIZE
KEYS
DEL name
GET name
EOF
```

The shell feeds each line to `redis-cli` as a separate command. You do not need to type `\r\n` yourself.

## 5. Values containing spaces

Quoted arguments are passed as a single value:

```bash
redis-cli -p 6379 SET greeting "hello world"
redis-cli -p 6379 GET greeting
redis-cli -p 6379 SET "greeting type" "blessing now"
redis-cli -p 6379 GET "greeting type"
```

> **Persistence limitation:** the persistence file format separates fields using whitespace. Keys or values containing spaces work while the server is running, but may not be restored correctly after a restart. Avoid spaces in keys/values during persistence tests until this is fixed.

## 6. INCR

```bash
redis-cli -p 6379 <<'EOF'
SET counter 5
INCR counter
GET counter
INCR counter
GET counter
EOF
```

`INCR` on a missing key:

```bash
redis-cli -p 6379 <<'EOF'
DEL newcounter
INCR newcounter
GET newcounter
EOF
```

`INCR` on a non-integer value:

```bash
redis-cli -p 6379 <<'EOF'
SET text hello
INCR text
GET text
EOF
```

## 7. EXPIRE and TTL

Set a key to expire after 10 seconds:

```bash
redis-cli -p 6379 <<'EOF'
SET session active
EXPIRE session 10
TTL session
GET session
EOF
```

Wait, then check the key:

```bash
sleep 11
redis-cli -p 6379 <<'EOF'
TTL session
GET session
EXISTS session
EOF
```

**Expected after expiration:** `TTL` returns `-2`, `GET` returns `(nil)`, and `EXISTS` returns false/zero.

### TTL return values

| Value | Meaning |
|---|---|
| Positive number | Seconds remaining (rounded down) |
| `-1` | Key exists but has no expiration |
| `-2` | Key does not exist or has expired |

### Immediate expiration

```bash
redis-cli -p 6379 <<'EOF'
SET temp value
EXPIRE temp 0
GET temp
EXISTS temp
TTL temp
EOF
```

### `SET` removes an existing TTL

```bash
redis-cli -p 6379 <<'EOF'
SET refresh old
EXPIRE refresh 60
TTL refresh
SET refresh new
TTL refresh
GET refresh
EOF
```

After overwriting the key with `SET`, `TTL` should return `-1`.

## 8. PERSIST

`PERSIST` removes an expiration but keeps the key:

```bash
redis-cli -p 6379 <<'EOF'
SET permanent hello
EXPIRE permanent 120
TTL permanent
PERSIST permanent
TTL permanent
GET permanent
EOF
```

**Expected after `PERSIST`:** `TTL` returns `-1`, and `GET` returns `"hello"`.

## 9. Expiration across a server restart

Set a key with a long enough TTL:

```bash
redis-cli -p 6379 SET restart_test hello
redis-cli -p 6379 EXPIRE restart_test 120
redis-cli -p 6379 TTL restart_test
```

Inspect the persistence file:

```bash
grep -E 'restart_test' ~/MiniRedis/data/database.txt
```

The file should contain a data record (`D`) and an expiration record (`E`) for the key.

In **Terminal 1**, press `Ctrl+C` to stop the server, then restart it:

```bash
cd ~/MiniRedis
./server
```

In **Terminal 2**, check the key:

```bash
redis-cli -p 6379 GET restart_test
redis-cli -p 6379 TTL restart_test
```

If the deadline has not passed, the value should be present and the TTL should be positive but lower than before the restart. The countdown continues while the server is stopped.

**Testing expiry while offline:** set a short TTL, stop the server for longer than that TTL, restart it, and check that `GET` returns `(nil)` and `TTL` returns `-2`.

## 10. DEL and CLEAR

Delete a single key:

```bash
redis-cli -p 6379 SET delete_check hello
redis-cli -p 6379 DEL delete_check
redis-cli -p 6379 GET delete_check
```

Clear the entire database (**removes all keys**):

```bash
redis-cli -p 6379 CLEAR
redis-cli -p 6379 SIZE
redis-cli -p 6379 KEYS
```

**Expected:** `SIZE` returns `0`, and `KEYS` reports an empty database.

## 11. CONFIG responses

```bash
redis-cli -p 6379 <<'EOF'
CONFIG GET save
CONFIG GET appendonly
CONFIG GET unknown
EOF
```

Only the documented `CONFIG GET` compatibility responses are implemented. The full Redis configuration system is not supported.

## 12. Testing with netcat

Send a RESP-encoded `PING`:

```bash
printf '*1\r\n$4\r\nPING\r\n' | nc localhost 6379
```

Expected:

```
+PONG
```

Two requests on one TCP connection:

```bash
printf '*1\r\n$4\r\nPING\r\n*1\r\n$4\r\nPING\r\n' | nc localhost 6379
```

Expected: two `+PONG` responses.

## 13. Benchmarking

With the server running:

```bash
redis-benchmark -p 6379
```

The benchmark may issue commands Mini Redis does not implement. Treat results as measurements of the supported workload, not a direct comparison with production Redis.

## 14. Inspect persisted data

```bash
cat ~/MiniRedis/data/database.txt
```

Record types:

| Record | Meaning |
|---|---|
| `D` | Key-value data |
| `E` | Expiration deadline |

This is an internal format. Do not edit it manually while the server is running.

## 15. Troubleshooting

| Symptom | Likely cause / fix |
|---|---|
| **Connection refused** | The server is not running or not listening on port 6379. Start it in Terminal 1. |
| **Address already in use** | Another process is using port 6379. Check for an existing server before starting another. |
| **unknown command** | Mini Redis supports only the commands listed in [README.md](README.md). It is not a full Redis replacement. |
| **`TTL` returns `-1`** | The key exists but has no expiration. Check whether `PERSIST` was called, or whether the key was restored without an `E` record. |
| **`TTL` returns `-2`** | The key does not exist or its expiration has passed. |
| **Key with spaces disappears/loads wrongly after restart** | Known limitation: the persistence format is whitespace-delimited. |

## 16. Stop and rebuild

Stop the server with `Ctrl+C` in Terminal 1.

Rebuild after code changes:

```bash
cd ~/MiniRedis
make clean
make
```

