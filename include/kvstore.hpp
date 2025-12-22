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
    bool set(const std::string &key,
             const std::string &value);

    // Retrieve a value by key
    std::optional<std::string> get(const std::string &key) const;

    // Delete a key
    bool del(const std::string &key);

    // Number of stored keys
    size_t size() const;

private:
    std::unordered_map<std::string, std::string> data_;
    mutable std::shared_mutex mutex_;

    size_t max_key_len_;
    size_t max_value_len_;
};
