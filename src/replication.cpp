#include <replication.hpp>
#include <kvstore.hpp>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

ReplicationManager::ReplicationManager(KVStore &store)
    : store_(store) {}



void ReplicationManager::leader_accept_loop(int port) {

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("replication socket");
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("replication bind");
        return;
    }

    if (listen(server_fd, 10) < 0) {
        perror("replication listen");
        return;
    }

    std::cout << "[REPL] leader listening on port " << port << std::endl;

    while (running_) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);

        int follower_fd = accept(server_fd, (sockaddr*)&client, &len);

        if (follower_fd < 0) {
            if (running_) {
                perror("replication accept");
            }
            continue;
        }

        follower_sockets_.emplace_back(follower_fd);

        std::cout << "[REPL] follower connected\n";
    }

    close(server_fd);
}



void ReplicationManager::start_leader(int port) {

    running_ = true;

    replication_thread_ = std::thread(
        &ReplicationManager::leader_accept_loop,
        this,
        port
    );
}

void ReplicationManager::start_follower(const std::string &leader_ip, int leader_port) {

    running_ = true;

    replication_thread_ = std::thread([this, leader_ip, leader_port] () {
        
        int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            perror("replication socket");
            return;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(leader_ip.c_str());
        server_addr.sin_port = htons(leader_port);
        
        int connect_to_server = connect(socket_fd, (sockaddr*)&server_addr, sizeof(server_addr));

        if (connect_to_server < 0) {
            perror("connect to server");
            return;
        }

        std::cout << "client connected to server\n";
        
        }
    );
}