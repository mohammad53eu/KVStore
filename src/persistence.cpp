#include "persistence.hpp"
#include "kvstore.hpp"
#include <fstream>
#include <vector>
#include <sstream>


PersistenceManager::PersistenceManager(const std::string& filename)
        : filename_(filename) {}


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
                file << "EX " << *ttl;
        }

        file << "\n";
}


void PersistenceManager::append_del(const std::string& key) {
        std::ofstream file(filename_, std::ios::app);

        if (!file.is_open()) {
                std::cerr << "Failed to open AOF file for writing\n";
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
