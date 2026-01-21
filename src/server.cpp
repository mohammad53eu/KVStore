#include "server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include <sstream>
#include <persistence.hpp>
#include "kvstore.hpp"


// initialize the class variables
TCPServer::TCPServer(int port, KVStore &store, PersistenceManager &file)
    : port_(port),
    server_fd_(-1),
    store_(store),
    file_(file),
    running_(false) {}


void TCPServer::start(std::atomic<bool> &running){
    
    // create the socket
    server_fd_= socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd_ < 0) {
        perror("socket");
        return;
    }

    // prepare address---
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));


    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0
    addr.sin_port = htons(port_);

    // bind socket to port
    if(bind(server_fd_, (sockaddr *)&addr, sizeof(addr)) < 0){
        perror("bind");
        close(server_fd_);
        return;
    }

    // start listening
    if(listen(server_fd_, 10) < 0) {
        perror("listen");
        close(server_fd_);
        return;
    }


    std::cout << "server listening on port " << port_ << std::endl;

    
    while(running){
        // select with timeout to check for incoming connections
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd_, &read_fds);
        
        timeval timeout;
        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;
        
        int result = select(server_fd_ + 1, &read_fds, nullptr, nullptr, &timeout);
        
        if(result < 0) {
            perror("select");
            break;
        }
        
        if(result == 0) {
            // timeout - no connection ready, loop back to check running flag
            continue;
        }
        
        // connection is ready to accept
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(
            server_fd_,
            (sockaddr *)&client_addr,
            &client_len
        );

        if(client_fd < 0) {
            perror("accept");
            continue;
        }

        std::thread(
            &TCPServer::handle_client,
            this,
            client_fd
        ).detach();
    } 
    close(server_fd_);
}

void TCPServer::handle_client(int client_fd) {
    std::cout << "Client connected (fd=" << client_fd << ")" << std::endl;


    char recv_buffer[1024];
    std::string data_buffer;

    while(true){
        ssize_t bytes = recv(client_fd, recv_buffer, sizeof(recv_buffer), 0);

        if(bytes <= 0){
            break;
        }

        data_buffer.append(recv_buffer, bytes);

        // process full line
        size_t pos;
        while((pos = data_buffer.find('\n')) != std::string::npos){
            std::string line = data_buffer.substr(0, pos);
            data_buffer.erase(0, pos + 1);

            handle_command(client_fd, line);
        }
    }

    std::cout << "Client disconnected (fd=" << client_fd << ")" << std::endl;
    close(client_fd);
}


/*
tokenizes a command line, respecting quoted strings with spaces and escape 
sequences (\\ and \"). returns a vector of parsed tokens suitable
for command processing.
*/
static std::vector<std::string> tokenize(const std::string &line) {
    
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;

    for(size_t i = 0; i < line.size(); i++){

        char c = line[i];

        // handle escape inside quotes
        if(c == '\\' && in_quotes && i + 1 < line.size()) {
            current += line[i + 1];
            i++;
        } else if(c == '"') {
            in_quotes = !in_quotes;
        } else if(c == ' ' && !in_quotes) {
            if(!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if(!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}


void TCPServer::handle_command(int client_fd, const std::string& line) {
    std::cout << "Received: [" << line << "]" << std::endl;

    auto tokens = tokenize(line);
    std::string token;


    if (tokens.empty()) {   
        return;
    }

    std::string cmd = tokens[0];
    std::string response;
    
    /*
    check whether the commad is SET / GET / DELETE.
    if not any of them return an error to the sender
    */
    if (cmd == "SET") {
    if (tokens.size() < 3) {
        response = "ERROR: SET requires a key and a value\n";
    } else {
        std::string key = tokens[1];

        std::string value;
        std::optional<int> ttl;

        size_t i = 2;

        // rebuild value until EX
        for (; i < tokens.size(); i++) {
            if (tokens[i] == "EX") {
                break;
            }
            if (!value.empty()) value += " ";
            value += tokens[i];
        }

        // parse TTL if present
        if (i < tokens.size()) {
            if (tokens[i] == "EX" && i + 1 < tokens.size()) {
                ttl = std::stoi(tokens[i + 1]);
            } else {
                response = "ERROR: invalid EX usage\n";
                send(client_fd, response.c_str(), response.size(), 0);
                return;
            }
        }
        file_.append_set(key, value, ttl);
        store_.set(key, value, ttl);
        response = "OK\n";
        }
        
    } else if(cmd == "GET"){
        if(tokens.size() < 2) {
            response = "ERROR: GET requires a key\n";
        } else {
            auto value = store_.get(tokens[1]);
            if(value) {
                response = *value + "\n";
            } else {
                response = "NULL\n";
            }
        }
    } else if(cmd == "DELETE") {
        if(tokens.size() < 2) {
            response = "ERROR: DELETE requires a key\n";
        } else {
            bool deleted = store_.del(tokens[1]);
            if (deleted) {
                file_.append_del(tokens[1]);
                response = "OK\n";
            } else {
                response = "NOT_FOUND\n";
            }
        }
    } else {
        response = "ERROR: unkown command\n";
    }

    send(client_fd, response.c_str(), response.size(), 0);
}
