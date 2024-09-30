/**
 * @file main.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Main file of IMAP client
 */

#include "imapclient.hpp"
#include "argparser.hpp"


int main(int argc, char *argv[]) {
    ArgParser args;
    try {
        args.parse(argv,argc);
    }

    catch (std::invalid_argument &e) {
        e.what();
        return 1;
    }
   
    Config config = args.getConfig();
    IMAPClient client(config);

    try {
        client.start();
    }

    catch(std::runtime_error &e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
