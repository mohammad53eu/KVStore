#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>


class KVStore;


class ReplicationManager {
    
    public:
        ReplicationManager(KVStore &store);

        void start_leader(int port);

        void start_follower(const std::string &leader_ip, int leader_port);

        void replicate_command(const std::string& command);

        void stop();

    private:
        KVStore& store_;
        std::vector<int> follower_sockets_;
        std::thread replication_thread_;
        std::atomic<bool> running_{false};

        void leader_accept_loop(int port);
        void follower_receive_loop(int socket_fd);
};