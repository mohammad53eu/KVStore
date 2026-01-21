#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <atomic>

class KVStore {
public:
    // Constructor with limits
    KVStore(size_t max_key_len = 1024,
            size_t max_value_len = 1 << 20);

    // Store a key-value pair
    struct Entry {
        std::string value;
        std::optional<std::chrono::steady_clock::time_point> expires_at; 
    };

    bool set(
        const std::string &key,
        const std::string &value,
        std::optional<int> ttl_seconds = std::nullopt
    );

    // Retrieve a value by key
    std::optional<std::string> get(const std::string &key);

    // Delete a key
    bool del(const std::string &key);

    // Number of stored keys
    size_t size() const;

    std::unordered_map<std::string, Entry> current_state() const;

    void start_cleanup_thread();

    void stop_cleanup_thread();


private:
    std::unordered_map<std::string, Entry> data_;
    mutable std::shared_mutex mutex_;

    size_t max_key_len_;
    size_t max_value_len_;

    std::thread cleaner_thread_;
    std::atomic<bool> stop_cleaner_{false};

    bool is_expired(const Entry& entry) const;
    void cleanup_expired();
};
