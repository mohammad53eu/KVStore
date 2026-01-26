# KVStore - High-Performance Key-Value Store

![Status](https://img.shields.io/badge/status-alpha-orange)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue)
![Platform](https://img.shields.io/badge/platform-linux-lightgrey)

A lightweight, thread-safe key-value storage system built in C++ with TCP networking support. Think of it as a simplified Redis - an in-memory database that stores data as key-value pairs accessible over the network.

## Overview

**KVStore** is an educational yet functional key-value store implementation demonstrating core concepts in systems programming, network architecture, and concurrent data structures. It provides a simple Redis-like interface for storing and retrieving data in memory.

### What is a Key-Value Store?

A key-value store is a NoSQL database that uses a simple key-value method to store data:
- **Key**: A unique identifier (e.g., "user:123", "session:abc")
- **Value**: Data associated with that key (user info, session data, configuration, etc.)

**Real-world examples:**
- **Redis**: Caching layer used by Twitter, GitHub, Stack Overflow
- **Amazon DynamoDB**: AWS's managed NoSQL database service
- **Memcached**: High-performance distributed memory caching system

## Features

### Current Implementation (Phase 1 & 2)
- ✅ **Core Operations**: GET, SET, DELETE commands
- ✅ **TCP Server**: Network-accessible server on configurable ports
- ✅ **Thread Safety**: Concurrent access using read-write locks (`std::shared_mutex`)
- ✅ **Size Limits**: Configurable maximum key (1KB) and value (1MB) sizes
- ✅ **Multiple Clients**: Handles concurrent client connections
- ✅ **Simple Protocol**: Text-based command protocol for easy debugging
- ✅ **TTL Support**: Automatic key expiration with configurable time-to-live
- ✅ **Persistence (AOF)**: Append-only file for data durability with automatic snapshots
- ✅ **Background Cleanup**: Automatic removal of expired keys every second
- ✅ **Replication**: Leader-Follower replication for high availability
- ✅ **Multi-threaded**: Separate threads for client handling, cleanup, persistence, and replication

### Planned Features (Future Phases)
- ⏳ **Data Types**: Lists, sets, sorted sets, hashes
- ⏳ **Clustering**: Distributed storage with consistent hashing
- ⏳ **Monitoring**: Metrics and health check endpoints
- ⏳ **Transaction Support**: MULTI/EXEC commands

## Architecture

### System Components

```
┌─────────────────┐          ┌─────────────────┐
│ TCP Client      │          │ TCP Client      │
│ (Leader:8000)   │          │ (Follower:7000) │
└────────┬────────┘          └────────┬────────┘
         │ Commands                   │ Commands
         ▼                            ▼
┌─────────────────────────────────────────────────┐
│              LEADER NODE (Port 8000)            │
├─────────────────────────────────────────────────┤
│ ┌─────────────┐  ┌──────────────────────────┐  │
│ │ TCP Server  │  │  Replication Server      │  │
│ │             │  │  (Port 8001)             │  │
│ └──────┬──────┘  └────────┬─────────────────┘  │
│        │                  │                     │
│        ▼                  │ Replicate           │
│ ┌─────────────────────────┴─────────────────┐  │
│ │           KVStore (Thread-Safe)           │  │
│ │  ┌─────────────────────────────────────┐  │  │
│ │  │  Hash Table (unordered_map)         │  │  │
│ │  │  + TTL Tracking                     │  │  │
│ │  └─────────────────────────────────────┘  │  │
│ └───────────────────┬───────────────────────┘  │
│                     │                           │
│        ┌────────────┼────────────┐              │
│        ▼            ▼            ▼              │
│   ┌─────────┐ ┌─────────┐ ┌──────────┐         │
│   │Cleanup  │ │Persist  │ │Replicate │         │
│   │Thread   │ │Thread   │ │Thread    │         │
│   │(1s)     │ │(15s)    │ │(realtime)│         │
│   └─────────┘ └────┬────┘ └────┬─────┘         │
└─────────────────────┼───────────┼───────────────┘
                      ▼           ▼
                 ┌─────────┐  ┌──────────────┐
                 │data.aof │  │  FOLLOWER    │
                 │  (AOF)  │  │  NODES       │
                 └─────────┘  │  (Port 8001) │
                              └──────────────┘
```

### Thread Safety

The KVStore uses `std::shared_mutex` for reader-writer locking:
- **Multiple readers** can access data simultaneously
- **Single writer** gets exclusive access during modifications
- Ensures data consistency without sacrificing read performance

### Threading Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    MAIN THREAD                          │
│  • Initialize components                                │
│  • Start background threads                             │
│  • Signal handling (SIGINT/SIGTERM)                     │
└──────────────────────┬──────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┬──────────────┐
        │              │              │              │
        ▼              ▼              ▼              ▼
┌──────────────┐ ┌──────────┐ ┌───────────┐ ┌────────────┐
│ TCP Server   │ │ Cleanup  │ │Persistence│ │Replication │
│   Thread     │ │  Thread  │ │  Thread   │ │  Thread    │
├──────────────┤ ├──────────┤ ├───────────┤ ├────────────┤
│• Accept      │ │• Every 1s│ │• Every 15s│ │• Leader:   │
│  connections │ │• Scan all│ │• Snapshot │ │  Accept    │
│• Spawn client│ │  keys    │ │  current  │ │  followers │
│  threads     │ │• Delete  │ │  state    │ │• Follower: │
│              │ │  expired │ │• Write to │ │  Sync with │
│   ┌──────┐   │ │  entries │ │  AOF file │ │  leader    │
│   │Client│   │ │          │ │• Replay   │ │• Replicate │
│   │Thread│   │ │          │ │  on start │ │  writes    │
│   └──────┘   │ │          │ │           │ │            │
│   ┌──────┐   │ │          │ │           │ │            │
│   │Client│   │ │          │ │           │ │            │
│   │Thread│   │ │          │ │           │ │            │
│   └──────┘   │ │          │ │           │ │            │
│     ...      │ │          │ │           │ │            │
└──────────────┘ └──────────┘ └───────────┘ └────────────┘
         │              │            │              │
         └──────────────┴────────────┴──────────────┘
                        │
                        ▼
              ┌──────────────────┐
              │     KVStore      │
              │  (shared_mutex)  │
              └──────────────────┘
```

**Thread Synchronization:**
- All threads access KVStore through shared_mutex (readers can run in parallel, writers get exclusive lock)
- Client threads: one per connection, handle commands
- Cleanup thread: periodically removes expired keys
- Persistence thread: snapshots data to disk every 15 seconds
- Replication thread: leader accepts followers and replicates writes in real-time

## Getting Started

### Prerequisites

- **Operating System**: Linux (tested on Fedora)
- **Compiler**: GCC/G++ with C++17 support
- **Build System**: CMake 3.15 or higher
- **Network Tools**: `telnet`, `nc` (netcat), or custom TCP client

### Building the Project

1. **Clone the repository**:
   ```bash
   git clone https://github.com/mohammad53eu/KVStore.git
   cd KVStore
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
SET <key> <value> [EX <seconds>]
```
**Examples:**
```
SET username john_doe
SET user:123 {"name":"John","age":30}
SET session:abc token123 EX 3600
```
**Response:** `OK` on success
**Optional TTL:** Use `EX <seconds>` to set expiration time

#### GET - Retrieve a value
```
GET <key>
```
**Example:**
```
GET username
```
**Response:** The stored value or `(NULL)` if not found or expired

#### DEL - Delete a key
```
DEL <key>
```
**Example:**
```
DEL username
```
**Response:** `OK` if deleted, `(NULL)` if key didn't exist

### Example Sessions

#### Basic Operations
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

#### Using TTL (Time-To-Live)
```bash
$ telnet localhost 8000

SET session:user123 active EX 10
OK
GET session:user123
active
# Wait 10 seconds...
GET session:user123
(NULL)
```

#### Running Leader-Follower Replication

**Terminal 1 - Start Leader:**
```bash
$ cd build
$ ./kvstore
# Leader starts on port 8000 (clients) and 8001 (replication)
```

**Terminal 2 - Start Follower:**
```bash
$ cd build
$ ./kvstore --follower 127.0.0.1 8001
# Follower starts on port 7000 (clients)
# Connects to leader at 127.0.0.1:8001
```

**Terminal 3 - Write to Leader:**
```bash
$ telnet localhost 8000
SET replicated_key hello_world
OK
```

**Terminal 4 - Read from Follower:**
```bash
$ telnet localhost 7000
GET replicated_key
hello_world
```

## Current Project Structure

```
kvstore/
├── CMakeLists.txt          # Build configuration
├── README.md               # Project documentation
├── LICENSE                 # MIT License
├── include/                # Header files
│   ├── kvstore.hpp        # Key-value store interface
│   ├── server.hpp         # TCP server interface
│   ├── persistence.hpp    # Persistence manager (AOF)
│   ├── replication.hpp    # Replication manager
│   └── node_role.hpp      # Node role enum (Leader/Follower)
├── src/                    # Implementation files
│   ├── kvstore.cpp        # KVStore implementation
│   ├── server.cpp         # TCP server implementation
│   ├── persistence.cpp    # Persistence implementation
│   ├── replication.cpp    # Replication implementation
│   └── main.cpp           # Entry point
└── build/                  # Build artifacts (generated)
    ├── kvstore            # Compiled executable
    └── data.aof           # Append-only file (persistence)
```

## Technical Details

### KVStore Class

```cpp
class KVStore {
public:
    KVStore(size_t max_key_len = 1024, size_t max_value_len = 1 << 20);
    
    bool set(const std::string &key, const std::string &value, 
             std::optional<int> ttl_seconds = std::nullopt);
    std::optional<std::string> get(const std::string &key);
    bool del(const std::string &key);
    size_t size() const;
    
    void start_cleanup_thread();  // Background TTL cleanup
    void stop_cleanup_thread();
};
```

- **Storage**: `std::unordered_map<std::string, Entry>` where Entry contains value and optional expiration time
- **Concurrency**: `std::shared_mutex` for thread-safe operations
- **Limits**: Max key size 1KB, max value size 1MB (configurable)
- **TTL**: Cleanup thread runs every 1 second to remove expired keys
- **Expiration Check**: Also validated during GET operations

### TCPServer Class

```cpp
class TCPServer {
public:
    TCPServer(int port, KVStore &store, PersistenceManager &file,
              NodeRole role, ReplicationManager &replica);
    void start(std::atomic<bool> &running);  // Blocking call
};
```

- **Protocol**: Plain text over TCP
- **Ports**: Leader=8000, Follower=7000 (client connections)
- **Multi-client**: Spawns threads for each connection
- **Blocking I/O**: Uses standard POSIX sockets
- **Signal Handling**: Graceful shutdown on SIGINT/SIGTERM

### PersistenceManager Class

```cpp
class PersistenceManager {
public:
    PersistenceManager(const KVStore &store, 
                       const std::string& filename = "data.aof");
    
    void append_set(const std::string& key, const std::string& value, 
                    std::optional<int> ttl);
    void append_del(const std::string& key);
    void replay(KVStore& store);  // Load from AOF on startup
    
    void start_save_state_thread();  // Background snapshots
    void stop_save_state_thread();
};
```

- **Format**: Append-only file (AOF) with text commands
- **Replay**: Reads and executes commands from file on startup
- **Snapshots**: Background thread saves full state every 15 seconds
- **TTL Preservation**: Stores and restores expiration times

### ReplicationManager Class

```cpp
class ReplicationManager {
public:
    ReplicationManager(KVStore &store, std::atomic<bool>& running);
    
    void start_leader(int port);  // Port 8001 for replication
    void start_follower(const std::string &leader_ip, int leader_port);
    void replicate_command(const std::string& command);
    void stop();
};
```

- **Leader**: Listens on port 8001, accepts follower connections
- **Follower**: Connects to leader, sends `SYNC\n` to initiate
- **Initial Sync**: Leader sends full snapshot (SNAPSHOT_BEGIN → SET commands → SNAPSHOT_END)
- **Real-time Replication**: All write operations replicated to followers immediately
- **Thread-safe**: Uses mutex to protect follower socket list

## Development Roadmap

### Phase 1: Basic In-Memory Store ✅
- [x] Core GET/SET/DELETE operations
- [x] TCP server with command parser
- [x] Thread-safe concurrent access
- [x] Basic error handling

### Phase 2: Persistence & Advanced Features ✅
- [x] Append-only file (AOF) for durability
- [x] Snapshot-based persistence (15-second intervals)
- [x] Time-to-live (TTL) for keys
- [x] Background cleanup thread (1-second intervals)
- [x] Replay mechanism for crash recovery
- [ ] Multiple data types (lists, sets, hashes)
- [ ] Transaction support (MULTI/EXEC)

### Phase 3: Distributed System 🚧
- [x] Leader-Follower replication
- [x] Real-time write replication
- [x] Initial snapshot synchronization
- [ ] Consistent hashing for data distribution
- [ ] Multi-leader cluster mode
- [ ] Raft consensus for leader election
- [ ] Automatic failover
- [ ] Monitoring dashboard and metrics

## Contributing

This is primarily a learning project, but feedback, suggestions, and issues are welcome! If you find a bug or have an idea for improvement:

1. **Open an issue** to discuss the problem or feature
2. **Fork the repository** and make your changes
3. **Submit a pull request** with a clear description

Feel free to use this project as a learning resource or starting point for your own key-value store implementation!

## Learning Objectives

This project demonstrates:

1. **Systems Programming**: Low-level C++ for high-performance applications
2. **Network Programming**: Socket programming, client-server architecture
3. **Concurrent Programming**: Mutexes, locks, thread-safe data structures
4. **Data Structures**: Hash tables, memory management
5. **Protocol Design**: Simple text-based network protocols
6. **Build Systems**: CMake, compilation, linking

## Performance Considerations

- **In-memory primary**: All data stored in RAM for maximum speed
- **Persistence overhead**: AOF writes every 15 seconds (background thread)
- **Hash table**: O(1) average-case for GET/SET/DEL operations
- **Read-write locks**: Multiple concurrent readers, exclusive writers (shared_mutex)
- **Connection overhead**: Each client spawns a new thread
- **TTL cleanup**: Scans all keys every second (may impact performance with large datasets)
- **Replication lag**: Minimal lag for writes (synchronous replication to followers)
- **Startup time**: Proportional to AOF file size (replay on startup)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

This is an educational project created for learning systems programming concepts. Feel free to use, modify, and learn from it!

## Resources

- [Redis Protocol Specification](https://redis.io/docs/latest/develop/reference/protocol-spec/)
- [POSIX Sockets Programming](https://beej.us/guide/bgnet/)
- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)

---

**Note**: This project is under active development. APIs and features may change as development progresses through the roadmap phases.