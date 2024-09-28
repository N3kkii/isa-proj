/**
 * @file imapcl.hpp
 * @author Vojtěch Adámek
 * 
 * @brief Implementation of IMAPClient class
 */

#ifndef IMAPCL_HPP
#define IMAPCL_HPP

#define BUFFER_SIZE 2048

class IMAPClient {
public:
    char buffer_in[BUFFER_SIZE]; // Buffer for incoming messages

    IMAPClient();
    ~IMAPClient();

    /**
     * @brief Starts the client
     * 
     * Creates socket for communication, resolves IP from domain
     */
    void start();

    /**
     * @brief Connects to the IMAP server using TCP
     */
    void connect();

    /**
     * @brief Handles incoming message from server
     * 
     * @return 1 if message is incomplete, else return 0;
     */
    int handleMessage();

private:

};


#endif