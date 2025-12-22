#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include "kvstore.hpp"


void writer(KVStore &store, int id, int iterations) {
    for (int i = 0; i < iterations; i++) {
        store.set(
            "key_" + std::to_string(id),
            "value_" + std::to_string(i)
        );
    }
}


void reader(const KVStore &store, int id, int iterations) {
    for (int i = 0; i < iterations; i++) {
        auto value = store.get("key_" + std::to_string(id));

    }
}


int main() {
    KVStore store;

    const int writer_threads = 4;
    const int reader_threads = 4;
    const int iterations = 10000;

    std::vector<std::thread> threads;

    // start writers
    for (int i = 0; i < writer_threads; i++) {
        threads.emplace_back(writer, std::ref(store), i, iterations);
    }

    // start readers
    for (int i = 0; i < reader_threads; i++) {
        threads.emplace_back(reader, std::cref(store), i, iterations);
    }

    // wait all threads
    for (auto &t : threads) {
        t.join();
    }

    std::cout << "Final size: " << store.size() << std::endl;

    // sanity check
    assert(store.size() <= writer_threads);

    std::cout << "Multithreaded test passed ✅" << std::endl;
    return 0;
}
