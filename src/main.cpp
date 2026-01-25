#include "kvstore.hpp"
#include "server.hpp"
#include "persistence.hpp"
#include <csignal>
#include <node_role.hpp>
#include <replication.hpp>

std::atomic<bool> running(true);

void handle_signal(int) {
    running = false;
}


int main(int argc, char* argv[]) {
    
    KVStore store;
    ReplicationManager replica(store);
    NodeRole role = NodeRole::Leader;
    if (argc >= 2 && std::string(argv[1]) == "--follower") {
        role = NodeRole::Follower;
        std::string leader_ip = (argc >= 3) ? argv[2] : "127.0.0.1";
        int leader_port = (argc >= 4) ? std::stoi(argv[3]) : 8000;
        replica.start_follower(leader_ip, leader_port);
    }
    
    int port = (role == NodeRole::Leader)? 8000: 7000;

    PersistenceManager file(store, "data.aof");
    TCPServer server(port, store, file, role, replica);
    file.replay(store);


    if (role == NodeRole::Leader) {
        replica.start_leader(8001);
    }

    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    
    file.start_save_state_thread();
    store.start_cleanup_thread();
    server.start(running);
    store.stop_cleanup_thread();
    file.stop_save_state_thread();

    return 0;
}
