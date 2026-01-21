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


void KVStore::cleanup_expired() {
    std:: unique_lock lock(mutex_);

    auto now = std::chrono::steady_clock::now();

    for (auto it = data_.begin(); it != data_.end();) {
        if (it ->second.expires_at && now >= *it ->second.expires_at) {
            it = data_.erase(it);

        } else {
            ++it;
        }
    }
}


void KVStore::start_cleanup_thread() {
    stop_cleaner_ = false;

    cleaner_thread_ = std::thread([this]() {
        while (!stop_cleaner_) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            cleanup_expired();
        }
    });
}


void KVStore::stop_cleanup_thread() {
    stop_cleaner_ = true;

    if (cleaner_thread_.joinable()) {
        cleaner_thread_.join();
        std::cout << "Stopped cleanup thread\n";
    }
}