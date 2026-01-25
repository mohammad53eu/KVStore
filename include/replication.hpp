#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>


class KVStore;


class ReplicationManager {
    
    public:
    ReplicationManager(KVStore &store, std::atomic<bool>& running);
    
    void start_leader(int port);
    
    void start_follower(const std::string &leader_ip, int leader_port);
    
    void replicate_command(const std::string& command);

    void apply_replicate_command(const std::string& command);

    
    void stop();
    
    private:
    KVStore& store_;
    std::atomic<bool>& running_;
    std::vector<int> follower_sockets_;
    std::thread replication_thread_;
    int server_fd_{-1};
    int follower_fd_{-1};
    std::mutex followers_mutex_;
    
        void leader_accept_loop(int port);
        void follower_receive_loop(const std::string& leader_ip, int leader_port);
};

