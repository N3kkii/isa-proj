/**
 * @file imapcl.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of IMAPClient class functionality
 */


#include "imapclient.hpp"
#include <fstream>

IMAPClient::IMAPClient(std::string &server, std::string &auth_file, std::string &out_dir, int port, std::string mailbox, 
                        std::string certfile, std::string certaddr, bool only_new, bool only_headers, bool secured):
    server{server},
    auth_file{auth_file},
    out_dir{out_dir},
    port{port},
    mailbox{mailbox},
    certfile{certfile},
    certaddr{certaddr},
    only_new{only_new},
    only_headers{only_headers},
    secured{secured},
    tag{1},
    logged{false}
{
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
    this->res = nullptr;
    this->sockfd = -1;
};

IMAPClient::IMAPClient(struct Config &config) :
    server{config.server},
    auth_file{config.auth_file},
    out_dir{config.out_dir},
    port{config.port},
    mailbox{config.mailbox},
    certfile{config.certfile},
    certaddr{config.certaddr},
    only_new{config.only_new},
    only_headers{config.only_headers},
    secured{config.secured},
    tag{1},
    logged{false}
{ 
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
    this->res = nullptr;
    this->sockfd = -1;
}

IMAPClient::~IMAPClient() {
    cleanup();
}

void IMAPClient::start() {
    this->connectToHost();
    this->login();
}


void IMAPClient::connectToHost() {

    // Initizalizing structures for getaddrinfo
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; 

    // Resolve domain name with getaddrinfo
    if (getaddrinfo(this->server.c_str(), std::to_string(this->port).c_str(), &hints, &this->res) != 0) {
        throw std::runtime_error("Error translating address");
    }

    // Create a communication socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Error creating socket.");
    }

    for (auto addr = this->res; addr != nullptr; addr = addr->ai_next) {
        // Try to connect, if successful, exit the loop
        // TODO Set the socket to nonblocking and set a timeout to avoid endless retransmission
        if (connect(this->sockfd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }

        // If addr is the last address, throw std::runtime_error
        if (addr->ai_next == nullptr) {
            throw std::runtime_error("Cannot connect to hostname");
        }
    }
    recv(this->sockfd, this->buffer_in, BUFFER_SIZE, 0);
    std::cout << this->buffer_in << std::endl;
}

void IMAPClient::login() {
    std::cout << "Trying to log in" << std::endl;
    std::ifstream file(this->auth_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open auth file.");
    }

    std::string username, password;
    
    if ((std::getline(file, username) && std::getline(file, password)) == false) {
        file.close();
        throw std::runtime_error("Wrong auth file format.");
    }

    file.close();
    this->sendCommand(std::string("LOGIN " + username + " " + password));
    recv(this->sockfd, this->buffer_in, BUFFER_SIZE, 0);
    std::cout << this->buffer_in << std::endl;
    this->logged = true;
}

void IMAPClient::logout() {
    this->sendCommand("LOGOUT");
    recv(this->sockfd, this->buffer_in, BUFFER_SIZE, 0);
    std::cout << this->buffer_in << std::endl;
}

void IMAPClient::sendCommand(const std::string &cmd) {
    std::string outstr = "A" + std::to_string(this->tag) + " " + cmd + " \r\n";
    if(send(this->sockfd, outstr.c_str(), outstr.length(), 0) < 0) {
        throw std::runtime_error("Failed to send a command");
    }
    this->tag++;
}

void IMAPClient::cleanup() {
    if (this->logged) {
        this->logout();
    }

    if (this->sockfd != -1){
        close(this->sockfd);
    }

    if (this->res != nullptr) {
     freeaddrinfo(res);
    }
}
