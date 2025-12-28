#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <mutex>

class KVStore {
public:
    // Constructor with limits
    KVStore(size_t max_key_len = 1024,
            size_t max_value_len = 1 << 20);

    // Store a key-value pair
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

private:
    struct Entry {
        std::string value;
        std::optional<std::chrono::steady_clock::time_point> expires_at; 
    };

    std::unordered_map<std::string, Entry> data_;
    mutable std::shared_mutex mutex_;

    size_t max_key_len_;
    size_t max_value_len_;


    bool is_expired(const Entry& entry) const;
};
