#include "kvstore.hpp"
#include "server.hpp"


int main() {
    KVStore store;
    TCPServer server(8000, store);

    server.start();

    
    return 0;
}
