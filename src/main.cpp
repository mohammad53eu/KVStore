#include "kvstore.hpp"
#include "server.hpp"
#include "persistence.hpp"
#include <csignal>

std::atomic<bool> running(true);

void handle_signal(int) {
    running = false;
}

int main() {
    KVStore store;
    PersistenceManager file("data.aof");
    TCPServer server(8000, store, file);
    
    file.replay(store);
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    store.start_cleanup_thread();
    server.start(running);
    store.stop_cleanup_thread();



    
    return 0;
}
