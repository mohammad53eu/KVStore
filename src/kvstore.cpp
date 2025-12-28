#include "kvstore.hpp"
#include <iostream>


KVStore::KVStore(size_t max_key_len, size_t max_value_len)
    : max_key_len_(max_key_len),
    max_value_len_(max_value_len){}


bool KVStore::set(
    const std::string &key,
    const std::string &value,
    std::optional<int> ttl_seconds
) {
    // size check
    Entry entry;
    entry.value = value;

    if (key.size() > max_key_len_ || value.size() > max_value_len_)
        return false;

    if (ttl_seconds) {
        entry.expires_at = 
            std::chrono::steady_clock::now() +
            std::chrono::seconds(*ttl_seconds);
    }
    std::unique_lock lock(mutex_);

    data_[key] = entry;
    return true;
}



std::optional<std::string> KVStore::get(const std::string &key) {

    std::unique_lock lock(mutex_);

    auto it = data_.find(key);
    
    if (it == data_.end()){
        return std::nullopt;
    }
    
    if (is_expired(it -> second)) {
        data_.erase(it);
        return std::nullopt;
    }
    return it->second.value;
}


bool KVStore::del(const std::string &key){

    std::unique_lock lock(mutex_);

    return data_.erase(key) > 0;
}


size_t KVStore::size() const {
    
    std::shared_lock lock(mutex_);

    return data_.size();
}

bool KVStore::is_expired(const Entry& entry) const {
    if (!entry.expires_at) {
        return false;
    }

    return std::chrono::steady_clock::now() >= *entry.expires_at;
}