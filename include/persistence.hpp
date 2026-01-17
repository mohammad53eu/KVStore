#pragma once

#include <iostream>
#include <optional>

/*
this is a forward declaration:
tells the compiler that there is a KVStore class somewhere - trust me :D
we do this to avoid heavy headers in .hpp (#include <kvstore.hpp>) - at least that is what i have been told
*/
class KVStore;

class PersistenceManager {
    public:
        PersistenceManager(const std::string& filename = "data.aof");

        void append_set(
            const std::string& key,
            const std::string& value,
            std::optional<int> ttl 
        );

        void append_del(const std::string& key);
        
        // replay the commands in the file
        void replay(KVStore& store);

    private:
        std:: string filename_;
};