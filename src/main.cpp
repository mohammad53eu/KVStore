#include "kvstore.hpp"
#include "server.hpp"
#include "persistence.hpp"

int main() {
    KVStore store;
    PersistenceManager file("data.aof");
    TCPServer server(8000, store, file);
    
    file.replay(store);

    server.start();

    
    return 0;
}
