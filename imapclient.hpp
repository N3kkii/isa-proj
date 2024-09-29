/**
 * @file imapcl.hpp
 * @author Vojtěch Adámek
 * 
 * @brief Implementation of IMAPClient class
 */

#ifndef IMAPCLIENT_HPP
#define IMAPCLIENT_HPP

#include "argparser.hpp"

// C++
#include <iostream>
#include <string>

// C
#include <string.h>
#include <unistd.h>

// Network libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 2048

class IMAPClient {
public:
    char buffer_in[BUFFER_SIZE]; // Buffer for incoming messages
    int sockfd;

    ArgParser args;

    IMAPClient(char *argv[], int argc);
    ~IMAPClient() = default;

    /**
     * @brief Starts the client
     * 
     * Creates socket for communication, resolves IP from domain
     */
    void start();

    /**
     * @brief Connects to the IMAP server using TCP
     */
    void connectToHost(std::string server);

    /**
     * @brief Handles incoming message from server
     * 
     * @return 1 if message is incomplete, else return 0;
     */
    int handleMessage();

private:

};


#endif
