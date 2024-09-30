/**
 * @file imapcl.hpp
 * @author Vojtěch Adámek
 * 
 * @brief Implementation of IMAPClient class
 */

#ifndef IMAPCLIENT_HPP
#define IMAPCLIENT_HPP

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

struct Config {
    std::string server;
    std::string auth_file;
    std::string out_dir;
    
    int port;
    std::string mailbox;
    std::string certfile;
    std::string certaddr;
    bool only_new;
    bool only_headers;
    bool secured;
};

class IMAPClient {
public:
    char buffer_in[BUFFER_SIZE]; // Buffer for incoming messages
    int sockfd;

    /**
     * @brief Construct a new IMAPClient object with provided parameters, optional parameteres have default values
     * 
     * For parameter description see class documentation
     */
    IMAPClient(std::string &server, std::string &auth_file, std::string &out_dir, int port = 143, 
                std::string mailbox = "INBOX", std::string certfile = "", std::string certaddr = "", 
                bool only_new = false, bool only_headers = false, bool secured = false);

    /**
     * @brief Construct a new IMAPClient object from a config structure
     * 
     * @param config structure with parsed command line arguments
     */
    IMAPClient(Config &config);
    
    /**
     * @brief Default destructor
     * 
     */
    ~IMAPClient() = default;

    /**
     * @brief Starts the client
     * 
     * Creates socket for communication, resolves IP from domain
     */
    void start();

private:
    std::string server;     // name (IP address) of server to connect to
    std::string auth_file;  // file with authentication credentials
    std::string out_dir;    // directory for storing downloaded mail
    
    int port;               // connecting port
    std::string mailbox;    // mailbox to work with
    std::string certfile;   // file with certificate for ssl
    std::string certaddr;   // directory with certificates
    bool only_new;          // work only with new mail
    bool only_headers;      // work only with mail headers
    bool secured;           // use tls

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

};


#endif
