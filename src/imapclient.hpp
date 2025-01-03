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
#include <vector>

// C
#include <string.h>
#include <unistd.h>

// Network libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

// SSL
#include  "openssl/bio.h"
#include  "openssl/ssl.h"
#include  "openssl/err.h"

#include "config.hpp"

#define BUFFER_SIZE 10000

enum class State {
    DISCONNECTED,
    CONNECTED,
    LOGGED,
    SELECTED,
    SEARCHING,
    FETCHING,
    LOGOUT
};


class IMAPClient {
public:
    char buffer_in[BUFFER_SIZE]; // Buffer for incoming messages

    /**
     * @brief Construct a new IMAPClient object with provided parameters, optional parameteres have default values
     * 
     * For parameter description see class documentation
     */
    IMAPClient(std::string &server, std::string &auth_file, std::string &out_dir, int port = 143, 
                std::string mailbox = "INBOX", std::string certfile = "", std::string certaddr = "/etc/ssl/certs", 
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
    ~IMAPClient();


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

    /* Variables for internal state */
    int tag;                // tag number for labeling outgoing commands
    State state;            // internal state of client
    bool complete;          // indicator of a complete response from a server for checkResponse() function
    bool uidvalidity;       // validity of mail UIDs
    bool synced;            // client - server synchronization flag
    std::string uidnext;
    std::string buff;       // input stream buffer
    std::vector<std::string> newuids; // vector of new message UIDs

    BIO *bio;               // OpenSSL BIO object for writing and reading on socket
    SSL_CTX *ctx;           // OpenSSL context structure

    unsigned long nmails;   // Number of downloaded mails


    /**
     * @brief Connects to the IMAP server using TCP
     */
    void connectToHost();


    /**
     * @brief Attemps to log in on the server
     */
    void login();


    /**
     * @brief Logs out the user
     * 
     */
    void logout();

    /**
     * @brief changes active mailbox
     * 
     */
    void selectMailbox();


    /**
     * @brief fetches mails from the server
     */
    void fetchMails();


    /**
     * @brief Constructs a tagged command to be sent to the server
     * 
     * @param cmd command to send
     */
    void sendCommand(const std::string &cmd);


    /**
     * @brief Checks the repsonse from the server
     */
    void checkResponse();

    /**
     * @brief Processes incoming data
     * 
     * @throw std::runtime_error if response is BAD/NO
     */
    void processResponse();


    /**
     * @brief Checks the tagged response code
     * 
     * @return  0 when response is OK,
     * @return  1 when response is NO,
     * @return -1 when response is BAD 
     */
    void checkTagged(const std::string response);


    /**
     * @brief Frees allocated memory and closes connection
     */
    void cleanup();
};


#endif
