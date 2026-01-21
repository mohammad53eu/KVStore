#include "persistence.hpp"
#include "kvstore.hpp"
#include <fstream>
#include <vector>
#include <sstream>


PersistenceManager::PersistenceManager(const KVStore &store, const std::string& filename)
        : store_(store),
          filename_(filename) {}


void PersistenceManager::append_set(
        const std::string& key,
        const std::string& value,
        std::optional<int> ttl
){
        std::ofstream file(filename_, std::ios::app);
        
        if (!file.is_open()) {
                std::cerr << "failed to open AOF fle for writing \n";

                return;
        }

        file << "SET " << key << " " << value;

        if(ttl){
                file << " EX " << *ttl;
        }

        file << "\n";
}


void PersistenceManager::append_del(const std::string& key) {
        std::ofstream file(filename_, std::ios::app);

        if (!file.is_open()) {
                std::cerr << "Failed to open data.aof file for writing\n";
                return;
        }

        file << "DELETE " << key << "\n";
}



void PersistenceManager::replay(KVStore& store) {

        std::ifstream file(filename_);

        if (!file.is_open()) {
                std::cout << "nothing saved in KVS, proceeding...\n";
                return;
        }

        std::string line;

        while(std::getline(file, line)) {
                if(line.empty()) continue;

                std::istringstream iss(line);
                std::vector<std::string> tokens;
                std::string token;

                while (iss >> token) {
                        tokens.emplace_back(token);
                }

                if (tokens.empty()) continue;

                const std::string& cmd = tokens[0];

                if ( cmd == "SET") {
                        if (tokens.size() < 3) continue;
                        std::string key = tokens[1];
                        std::string value;
                        std::optional<int> ttl;

                        size_t i = 2;

                        for (; i < tokens.size(); i++) {
                                if(tokens[i] == "EX") break;
                                if(!value.empty()) value += " ";
                                value += tokens[i];
                        }

                        if (i < tokens.size() && tokens[i] == "EX" && i + 1 < tokens.size()) {
                                try {
                                        ttl = std::stoi(tokens[i + 1]);
                                } catch (...) {
                                        continue;
                                }
                        }

                        store.set(key, value, ttl);
                }

                else if (cmd == "DELETE") {
                        if (tokens.size() >= 2) {
                                store.del(tokens[1]);
                        }
                }       
        }
}


void PersistenceManager::save_state() {

        std::string temp_file = filename_ + ".temp";
        std::ofstream file(temp_file, std::ios::trunc);

        if (!file.is_open()) {
                std::cerr << "Failed to open data.aof.temp for writing\n";
                return;
        }

        const auto& data = store_.current_state();

        if (data.empty()) {
                std::cout << "no data to store in .temp file";
                return;
        }
        
        for (const auto & pair: data) {
                file << "SET " << pair.first << " " << pair.second.value;
                
                if (pair.second.expires_at) {
                        auto now = std::chrono::steady_clock::now();
                        auto ttl = std::chrono::duration_cast<std::chrono::seconds>(
                                *pair.second.expires_at - now
                        ).count();
                        
                        if (ttl > 0) {
                                file << " EX " << ttl;
                        }
                }

                file << "\n";
        }

        file.close();

        std::rename(temp_file.c_str(), filename_.c_str());
}


void PersistenceManager::start_save_state_thread() {

        thread_should_stop_ = false;
        /* 
        will work on it later - the idea is to check the file size
        and other things to decide if rewriting is worth it
        */
        // std::fstream file(filename_, std::ios::ate);

        save_state_thread_ = std::thread([this]() {
                while(!thread_should_stop_) {
                        std::this_thread::sleep_for(std::chrono::seconds(15));
                        PersistenceManager::save_state();
                }
        });
}


void PersistenceManager::stop_save_state_thread() {
        thread_should_stop_ = true;

        if (save_state_thread_.joinable()) {
                save_state_thread_.join();
                std::cout << "Stopped save_state thread\n";

        }
}