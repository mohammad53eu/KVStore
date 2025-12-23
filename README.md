# KVStore - High-Performance Key-Value Store

A lightweight, thread-safe key-value storage system built in C++ with TCP networking support. Think of it as a simplified Redis - an in-memory database that stores data as key-value pairs accessible over a network.

## Overview

**KVStore** is an educational yet functional key-value store implementation demonstrating core concepts in systems programming, network architecture, and concurrent data structures. It provides a simple text-based protocol for storing, retrieving, and deleting string data.

### What is a Key-Value Store?

A key-value store is a NoSQL database that uses a simple key-value method to store data:
- **Key**: A unique identifier (e.g., "user:123", "session:abc")
- **Value**: Data associated with that key (user info, session data, configuration, etc.)

**Real-world examples:**
- **Redis**: Caching layer used by Twitter, GitHub, Stack Overflow
- **Amazon DynamoDB**: AWS's managed NoSQL database service
- **Memcached**: High-performance distributed memory caching system

## Features

### Current Implementation (Phase 1)
- ✅ **Core Operations**: GET, SET, DELETE commands
- ✅ **TCP Server**: Network-accessible server on port 8000
- ✅ **Thread Safety**: Concurrent access using read-write locks (`std::shared_mutex`)
- ✅ **Size Limits**: Configurable maximum key (1KB) and value (1MB) sizes
- ✅ **Multiple Clients**: Handles concurrent client connections
- ✅ **Simple Protocol**: Text-based command protocol for easy debugging

### Planned Features (Future Phases)
- ⏳ **Persistence**: Disk-based storage with snapshots and append-only logs
- ⏳ **TTL Support**: Automatic key expiration
- ⏳ **Data Types**: Lists, sets, sorted sets, hashes
- ⏳ **Replication**: Master-slave replication for high availability
- ⏳ **Clustering**: Distributed storage with consistent hashing
- ⏳ **Monitoring**: Metrics and health check endpoints

## Architecture

### Components

```
┌─────────────────┐
│   TCP Client    │
│  (telnet/nc)    │
└────────┬────────┘
         │ Commands (SET/GET/DEL)
         ▼
┌─────────────────┐
│   TCP Server    │
│   (Port 8000)   │
├─────────────────┤
│ Command Parser  │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│    KVStore      │
│ (Thread-Safe)   │
├─────────────────┤
│  Hash Table     │
│  (unordered_map)│
│  Shared Mutex   │
└─────────────────┘
```

### Thread Safety

The KVStore uses `std::shared_mutex` for reader-writer locking:
- **Multiple readers** can access data simultaneously
- **Single writer** gets exclusive access during modifications
- Ensures data consistency without sacrificing read performance

## Getting Started

### Prerequisites

- **Operating System**: Linux (tested on Fedora)
- **Compiler**: GCC/G++ with C++17 support
- **Build System**: CMake 3.15 or higher
- **Network Tools**: `telnet`, `nc` (netcat), or custom TCP client

### Building the Project

1. **Clone the repository** (if using Git):
   ```bash
   git clone <repository-url>
   cd kvstore
   ```

2. **Create build directory**:
   ```bash
   mkdir -p build
   cd build
   ```

3. **Configure with CMake**:
   ```bash
   cmake ..
   ```

4. **Compile**:
   ```bash
   make
   ```


## Usage

### Starting the Server

```bash
cd build
./kvstore
```

The server starts and listens on port 8000. It runs in the foreground and accepts multiple concurrent connections.

### Connecting to the Server

Use any TCP client to connect:

**Using telnet:**
```bash
telnet localhost 8000
```

**Using netcat:**
```bash
nc localhost 8000
```

### Command Protocol

Commands are text-based, one per line:

#### SET - Store a key-value pair
```
SET <key> <value>
```
**Example:**
```
SET username john_doe
SET user:123 {"name":"John","age":30}
```
**Response:** `OK` on success

#### GET - Retrieve a value
```
GET <key>
```
**Example:**
```
GET username
```
**Response:** The stored value or `(NULL)` if not found

#### DEL - Delete a key
```
DEL <key>
```
**Example:**
```
DEL username
```
**Response:** `OK` if deleted, `(NULL)` if key didn't exist

### Example Session

```bash
$ telnet localhost 8000
Trying 127.0.0.1...
Connected to localhost.

SET mykey hello
OK
GET mykey
hello
SET counter 42
OK
GET counter
42
DEL mykey
OK
GET mykey
(NULL)
```

## Current Project Structure

```
kvstore/
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── include/                # Header files
│   ├── kvstore.hpp        # Key-value store interface
│   └── server.hpp         # TCP server interface
├── src/                    # Implementation files
│   ├── kvstore.cpp        # KVStore implementation
│   ├── server.cpp         # TCP server implementation
│   └── main.cpp           # Entry point
└── build/                  # Build artifacts (generated)
    └── kvstore            # Compiled executable
```

## Technical Details

### KVStore Class

```cpp
class KVStore {
public:
    KVStore(size_t max_key_len = 1024, size_t max_value_len = 1 << 20);
    
    bool set(const std::string &key, const std::string &value);
    std::optional<std::string> get(const std::string &key) const;
    bool del(const std::string &key);
    size_t size() const;
};
```

- **Storage**: `std::unordered_map<std::string, std::string>`
- **Concurrency**: `std::shared_mutex` for thread-safe operations
- **Limits**: Max key size 1KB, max value size 1MB (configurable)

### TCPServer Class

```cpp
class TCPServer {
public:
    TCPServer(int port, KVStore &store);
    void start();  // Blocking call, accepts connections
};
```

- **Protocol**: Plain text over TCP
- **Port**: 8000 (default)
- **Multi-client**: Spawns threads for each connection
- **Blocking I/O**: Uses standard POSIX sockets

## Development Roadmap

### Phase 1: Basic In-Memory Store ✅
- [x] Core GET/SET/DELETE operations
- [x] TCP server with command parser
- [x] Thread-safe concurrent access
- [x] Basic error handling

### Phase 2: Persistence & Advanced Features
- [ ] Append-only file (AOF) for durability
- [ ] Snapshot-based persistence (RDB)
- [ ] Time-to-live (TTL) for keys
- [ ] Multiple data types (lists, sets, hashes)
- [ ] Transaction support (MULTI/EXEC)

### Phase 3: Distributed System
- [ ] Master-slave replication
- [ ] Consistent hashing for data distribution
- [ ] Cluster mode with multiple nodes
- [ ] Raft consensus for leader election
- [ ] Monitoring dashboard and metrics

## Learning Objectives

This project demonstrates:

1. **Systems Programming**: Low-level C++ for high-performance applications
2. **Network Programming**: Socket programming, client-server architecture
3. **Concurrent Programming**: Mutexes, locks, thread-safe data structures
4. **Data Structures**: Hash tables, memory management
5. **Protocol Design**: Simple text-based network protocols
6. **Build Systems**: CMake, compilation, linking

## Performance Considerations

- **In-memory only**: All data stored in RAM for maximum speed
- **No persistence yet**: Data lost on server restart
- **Linear search**: O(1) average-case for GET/SET/DEL operations
- **Read-write locks**: Multiple concurrent readers, exclusive writers
- **Connection overhead**: Each client spawns a new thread

## License

This is an educational project. Feel free to use and modify for learning purposes.

## Resources

- [Redis Protocol Specification](https://redis.io/docs/latest/develop/reference/protocol-spec/)
- [POSIX Sockets Programming](https://beej.us/guide/bgnet/)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)