# Minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project name and version
project(KVStore VERSION 1.0.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories (where header files are)
include_directories(${PROJECT_SOURCE_DIR}/include)

# Create executable from source files
add_executable(kvstore 
    src/main.cpp
    src/kvstore.cpp
    src/server.cpp
    src/persistence.cpp
    src/replication.cpp
)