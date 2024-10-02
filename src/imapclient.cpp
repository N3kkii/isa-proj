/**
 * @file imapcl.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of IMAPClient class functionality
 */


#include "imapclient.hpp"

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
    secured{secured}
{
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
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
    secured{config.secured}
{ 
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
}

void IMAPClient::start() {
    this->connectToHost(this->server);
}


void IMAPClient::connectToHost(std::string server) {
    // Initizalizing structures for getaddrinfo
    addrinfo *res, hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; 

    // Resolve domain name with getaddrinfo
    if (getaddrinfo(server.c_str(), "imap", &hints, &res) != 0) {
        throw std::runtime_error("Error translating address");
    }

    // Create a communication socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Error creating socket.");
    }

    for (auto addr = res; addr != nullptr; addr = addr->ai_next) {
        // Try to connect, if successful, exit the loop
        if (connect(this->sockfd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }

        // If addr is the last address, throw std::runtime_error
        if (addr->ai_next == nullptr) {
            throw std::runtime_error("Cannot connect to hostname");
            close(this->sockfd);
            freeaddrinfo(res);
        }
    }

    // Cleanup
    close(sockfd);
    freeaddrinfo(res);
}
