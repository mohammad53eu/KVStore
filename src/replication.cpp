#include <replication.hpp>
#include <kvstore.hpp>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <unordered_map>

ReplicationManager::ReplicationManager(KVStore &store, std::atomic<bool>& running)
    : store_(store), running_(running) {}



void ReplicationManager::leader_accept_loop(int port) {

    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        perror("replication socket");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("replication bind");
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        perror("replication listen");
        return;
    }

    std::cout << "[REPL] leader listening on port " << port << std::endl;

    while (running_) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);

        int follower_fd = accept(server_fd_, (sockaddr*)&client, &len);

        if (follower_fd < 0) {
            if (running_) {
                perror("replication accept");
            }
            continue;
        }

        std::cout << "[REPL] follower connected\n";
        
        char buffer[128];
        size_t n = recv(follower_fd, buffer, sizeof(buffer), 0);
        
        if (n <= 0) {
            close(follower_fd);
            return;
        }

        std::string msg(buffer, n);

        if (msg != "SYNC\n") {
            close(follower_fd);
            return;
        }

        auto snapshot = store_.current_state_leader();

        send(follower_fd, "SNAPSHOT_BEGIN\n", 15, 0);
    
        for (const auto& e : snapshot) {
            std::string cmd = "SET " + e.key + " " + e.value;
        
            if (e.ttl_seconds) {
                cmd += (" EX " + std::to_string(*e.ttl_seconds));
            }

            cmd += ("\n");
            send(follower_fd, cmd.c_str(), cmd.size(), 0);
        }
    
        send(follower_fd, "SNAPSHOT_END\n", 13, 0);

        std::lock_guard<std::mutex> lock(followers_mutex_);

        follower_sockets_.emplace_back(follower_fd);
    }

    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
}


void ReplicationManager::replicate_command(const std::string& command) {

    std::lock_guard<std::mutex> lock(followers_mutex_);

    for (auto it = follower_sockets_.begin(); it != follower_sockets_.end();) {
        
        int fd = *it;

        ssize_t sent = send(fd, command.c_str(), command.size(), 0);

        if (sent <= 0) {
            close(fd);
            it = follower_sockets_.erase(it);
            std::cout << "[REPL] follower disconnected\n";
        } else {
            ++it;
        }
    }
}


void ReplicationManager::start_leader(int port) {
    replication_thread_ = std::thread(
        &ReplicationManager::leader_accept_loop,
        this,
        port
    );
}


void ReplicationManager::apply_replicate_command(const std::string& command) {

    std::istringstream iss(command);
    std::string cmd, key;

    iss >> cmd;

    if (cmd == "SET") {
        iss >> key;

        std::string value, ex_keyword;
        iss >> value;
        iss >> ex_keyword;

        // Check if TTL is present
        if (ex_keyword == "EX") {
            int ttl;
            iss >> ttl;
            store_.set(key, value, ttl);
        } else {
            store_.set(key, value);
        }
    } else if (cmd == "DELETE") {
        iss >> key;

        store_.del(key);
    }
}


void ReplicationManager::follower_receive_loop(const std::string& leader_ip, int leader_port) {
    while (running_) {
        follower_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (follower_fd_ < 0) {
            perror("replication socket");
            return;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(leader_ip.c_str());
        server_addr.sin_port = htons(leader_port);
        
        int connect_to_server = connect(follower_fd_, (sockaddr*)&server_addr, sizeof(server_addr));

        if (connect_to_server < 0) {
            perror("connect to leader");
            return;
        }
        const char* sync = "SYNC\n";
        send(follower_fd_, sync, strlen(sync), 0);

        std::cout << "client connected to leader\n";

        bool syncing = true;

        while (running_) {
            char buffer[1024];
            ssize_t bytes = recv(follower_fd_, buffer, sizeof(buffer), 0);

            if (bytes < 0) {
                std::cout << "[REPL] connection lost\n";
                break;
            }

            std::string command(buffer, bytes);
            std::istringstream stream(command);
            std::string line;

            while (std::getline(stream, line)) {

                if (line == "SNAPSHOT_BEGIN") {
                    syncing = true;
                    continue;
                }

                if (line == "SNAPSHOT_END") {
                    syncing = false;
                    std::cout << "[REPL] snapshot complete\n";
                    continue;
                }

                apply_replicate_command(command);
            }
        }

        if (follower_fd_ >= 0) {
            close(follower_fd_);
            follower_fd_ = -1;
        }
    }
}

void ReplicationManager::start_follower(const std::string &leader_ip, int leader_port) {
    replication_thread_ = std::thread(
        &ReplicationManager::follower_receive_loop,
        this,
        leader_ip,
        leader_port
    );
}

void ReplicationManager::stop() {
    running_ = false;
    
    // close server socket to unblock accept()
    if (server_fd_ >= 0) {
        shutdown(server_fd_, SHUT_RDWR);
        close(server_fd_);
        server_fd_ = -1;
    }
    
    // close follower socket to unblock recv()
    if (follower_fd_ >= 0) {
        shutdown(follower_fd_, SHUT_RDWR);
        close(follower_fd_);
        follower_fd_ = -1;
    }
    
    // close all follower sockets
    {
        std::lock_guard<std::mutex> lock(followers_mutex_);
        for (int fd : follower_sockets_) {
            shutdown(fd, SHUT_RDWR);
            close(fd);
        }
        follower_sockets_.clear();
    }
    
    if (replication_thread_.joinable()) {
        replication_thread_.join();
        std::cout << "stopped replication thread\n";
    }
}