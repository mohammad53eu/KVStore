#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <netinet/in.h>
#include "kvstore.hpp"

class TCPServer {
public:
    // Create server listening on given port
    TCPServer(int port, KVStore &store);

    // Start accepting clients (blocking)
    void start();

private:
    // Handle one connected client
    void handle_client(int client_fd);

    // Parse and execute a command
    std::string process_command(const std::string &line);

    
    void handle_command(int client_fd, const std::string& line);


private:
    int port_;
    int server_fd_;
    KVStore &store_;
    std::atomic<bool> running_;
};
