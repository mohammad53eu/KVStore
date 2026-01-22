#include "kvstore.hpp"
#include "server.hpp"
#include "persistence.hpp"
#include <csignal>
#include <node_role.hpp>

std::atomic<bool> running(true);

void handle_signal(int) {
    running = false;
}

NodeRole role = NodeRole::Leader;

int main(int argc, char* argv[]) {
    if (argc >= 2 && std::string(argv[1]) == "--follower") {
    role = NodeRole::Follower;
    }
    KVStore store;
    PersistenceManager file(store, "data.aof");
    TCPServer server(8000, store, file, role);
    
    file.replay(store);




    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    
    file.start_save_state_thread();
    store.start_cleanup_thread();
    server.start(running);
    store.stop_cleanup_thread();
    file.stop_save_state_thread();


    
    return 0;
}
