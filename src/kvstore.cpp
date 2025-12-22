#include "kvstore.hpp"


KVStore::KVStore(size_t max_key_len, size_t max_value_len)
    : max_key_len_(max_key_len),
    max_value_len_(max_value_len){}


bool KVStore::set(
    const std::string &key,
    const std::string &value
) {
    // size check
    if(key.size() > max_key_len_ || value.size() > max_value_len_)
        return false;

    std::unique_lock lock(mutex_);

    data_[key] = value;
    return true;
}


std::optional<std::string> KVStore::get(const std::string &key) const {

    std::shared_lock lock(mutex_);

    auto it = data_.find(key);
    
    if(it == data_.end())
        return std::nullopt;
    
    return it->second;
}


bool KVStore::del(const std::string &key){

    std::unique_lock lock(mutex_);

    return data_.erase(key) > 0;
}


size_t KVStore::size() const {
    
    std::shared_lock lock(mutex_);

    return data_.size();
}