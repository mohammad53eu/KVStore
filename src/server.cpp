#include "server.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>
#include <sstream>


TCPServer::TCPServer(int port, KVStore &store)
    : port_(port),
    server_fd_(-1),
    store_(store),
    running_(false) {}


void TCPServer::start(){
    
    // create the socket
    server_fd_= socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd_ < 0) {
        perror("socket");
        return;
    }

    // prepare address
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


    running_ = true;

    std::cout << "server listening on port " << port_ << std::endl;

    while(running_){
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


void TCPServer::handle_command(int client_fd, const std::string& line) {
    std::cout << "Received: [" << line << "]" << std::endl;

    std::istringstream iss(line);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {   
        return;
    }

    std::string cmd = tokens[0];
    std::string response;

    if(cmd == "SET"){
        if(tokens.size() < 3) {
            response = "ERROR: SET requires a key and a value\n";
        } else {
            store_.set(tokens[1], tokens[2]);
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
            response = deleted ? "OK\n" : "NOT_FOUND\n";
        }
    } else {
        response = "ERROR: unkown command\n";
    }

    send(client_fd, response.c_str(), response.size(), 0);
}
