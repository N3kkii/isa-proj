/**
 * @file main.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Main file of IMAP client
 */

#include "imapclient.hpp"


int main(int argc, char *argv[]) {
    IMAPClient client{argv, argc};
    try {
        client.start();
    }

    catch(std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
