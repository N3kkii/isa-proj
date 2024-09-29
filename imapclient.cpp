/**
 * @file imapcl.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of IMAPClient class functionality
 */


#include "imapclient.hpp"


IMAPClient::IMAPClient(char *argv[], int argc) {
    buffer_in[0] = {};
    args.parse(argv, argc);
};

void IMAPClient::start() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket." << std::endl;
        exit(1);
    }

    this->connectToHost(this->args.server);
}


void IMAPClient::connectToHost(std::string server) {
    // Resolve domain with DNS
    addrinfo *res, hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; 

    if (getaddrinfo(server.c_str(), "imap", &hints, &res) != 0) {
        close(sockfd);
        throw std::runtime_error("Error translating address, terminating");
    }

    // Connect to the host
    connect(this->sockfd, res->ai_addr, res->ai_addrlen);
    std::cout << "Connection estabilished." << std::endl;
    std::cout << "Terminating connection." << std::endl;
    close(sockfd);
    freeaddrinfo(res);
}