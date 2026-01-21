#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <netinet/in.h>


class KVStore;
class PersistenceManager;

class TCPServer {
public:
    // Create server listening on given port
    TCPServer(int port, KVStore &store, PersistenceManager &file);

    // Start accepting clients (blocking)
    void start(std::atomic<bool> &running);

    // Handle one connected client
    void handle_client(int client_fd);
    
    
    
    private:
    void handle_command(int client_fd, const std::string& line);

    PersistenceManager &file_;
    int port_;
    int server_fd_;
    KVStore &store_;
    // i am leaving it for now
    std::atomic<bool> running_;
};
