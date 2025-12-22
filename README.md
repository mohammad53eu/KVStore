# KVStore - Distributed Key-Value Store

## Project Description

**KVStore** is a high-performance, distributed key-value storage system built from scratch in C++. Think of it as a mini-Redis or a simplified DynamoDB - a fast in-memory database that stores data as key-value pairs and can be accessed over a network.

### What is a Key-Value Store?

A key-value store is like a giant hash map/dictionary that lives on a server:
- **Key**: A unique identifier (like "user:123" or "session:abc")
- **Value**: Any data associated with that key (user info, session data, etc.)

Real-world examples:
- **Redis**: Used by Twitter, GitHub, Stack Overflow for caching
- **DynamoDB**: Amazon's own key-value database powering AWS
- **Memcached**: Used by Facebook, YouTube for fast data access

### Why Build This?

This project demonstrates critical skills that Amazon and other tech companies value:

1. **Systems Programming**: Low-level C++ for performance-critical applications
2. **Network Programming**: Building client-server architecture from scratch
3. **Distributed Systems**: Understanding replication, consistency, and fault tolerance
4. **Scalability**: Designing systems that handle thousands of requests
5. **Data Structures**: Implementing efficient storage and retrieval mechanisms

### Project Phases

#### Phase 1: Basic In-Memory Store (Current)
- Core GET/SET/DELETE operations
- Simple TCP server
- Command-line client
- Thread-safe operations

#### Phase 2: Persistence & Advanced Features
- Disk persistence (survive restarts)
- Key expiration/TTL
- Multiple data types (strings, lists, sets)
- Basic replication

#### Phase 3: Distributed System
- Multiple server nodes
- Consistent hashing for data distribution
- Leader election
- Replication and fault tolerance
- Monitoring and metrics

### Technical Architecture

**Components we'll build:**
- **Storage Engine**: In-memory hash table with thread-safe access
- **Network Layer**: TCP server handling multiple client connections
- **Protocol**: Simple text-based command protocol (like Redis)
- **Client Library**: C++ client for interacting with the server
- **Replication System**: Data sync across multiple nodes (Phase 3)

### Learning Outcomes

By building this project, you'll gain hands-on experience with:
- **C++ Modern Features**: Smart pointers, move semantics, lambda functions
- **Concurrency**: Mutexes, condition variables, thread pools
- **Networking**: Sockets, TCP/IP, client-server communication
- **Data Structures**: Hash tables, concurrent data structures
- **System Design**: Thinking about performance, reliability, and scalability

---

## Project Initialization

### Development Environment

**Operating System**: Fedora Linux  
**Compiler**: GCC/G++ (GNU Compiler Collection)  
**Build System**: CMake 3.15+  
**Version Control**: Git

### Initial Setup Steps

#### 1. Installed Required Tools

We verified/installed the essential development tools:

```bash
# C++ compiler (already installed)
g++ --version

# CMake build system
sudo dnf install cmake
cmake --version

# Make build tool
sudo dnf install make
make --version
```

#### 2. Created Project Structure

Set up a clean, organized project directory:

```bash
mkdir kvstore
cd kvstore

# Create folder structure
mkdir src       # Source files (.cpp)
mkdir include   # Header files (.hpp)
mkdir build     # Build artifacts (gitignored)
```

**Final structure:**
```
kvstore/
├── CMakeLists.txt    # Build configuration
├── .gitignore        # Git ignore rules
├── src/              # Implementation files
│   └── main.cpp
├── include/          # Header files
│   └── server.hpp
└── build/            # Compiled output (gitignored)
```

#### 3. Configured Build System (CMakeLists.txt)

Created `CMakeLists.txt` as the project's build configuration:

```cmake
cmake_minimum_required(VERSION 3.15)
project(KVStore VERSION 1.0.0)

# Use C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include header directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Build executable
add_executable(kvstore 
    src/main.cpp
)
```

**What this does:**
- Sets minimum CMake version requirement
- Declares project name and version
- Enforces C++17 standard (modern C++ features)
- Tells compiler where to find header files
- Defines which source files to compile and output name

#### 4. Version Control Setup

Initialized Git repository with appropriate ignore rules:

```bash
git init
# Added .gitignore from GitHub C++ template
```

The `.gitignore` excludes:
- `build/` directory (compiled artifacts)
- Object files (`.o`)
- Executables
- IDE-specific files

#### 5. Hello World Test

Created initial `src/main.cpp` to verify build system:

```cpp
#include <iostream>

int main() {
    std::cout << "Hello from KVStore!" << std::endl;
    return 0;
}
```

**Build and run process:**
```bash
cd build
cmake ..        # Configure: read CMakeLists.txt, generate build files
make           # Compile: turn .cpp into executable
./kvstore      # Run: execute the program
```

**Output:** `Hello from KVStore!` ✓

### Build System Workflow

The typical development cycle:

1. **First time setup:**
   ```bash
   cd build
   cmake ..
   ```

2. **After code changes:**
   ```bash
   make              # Recompile
   ./kvstore         # Run
   ```

3. **Clean rebuild (if needed):**
   ```bash
   rm -rf build/*
   cmake ..
   make
   ```

### Key Concepts

**CMake vs Make:**
- **CMake**: Configuration tool, generates build instructions (run once or when CMakeLists.txt changes)
- **Make**: Build tool, executes compilation (run after every code change)

**Header vs Source files:**
- **Headers (.hpp)**: Declarations, interfaces, what exists
- **Source (.cpp)**: Implementations, actual code, how it works

**Build Directory:**
- Keeps compiled files separate from source code
- Easy to clean (just delete build/)
- Standard practice in C++ projects

---

## Next Steps

Now that the project is initialized, we'll begin implementing Phase 1:
1. Design the storage engine (in-memory hash table)
2. Create the TCP server
3. Implement basic commands (GET, SET, DELETE)
4. Build a simple client to test it

---

**Status**: ✅ Project initialized and verified  
**Current Phase**: Beginning Phase 1 - Basic In-Memory Store