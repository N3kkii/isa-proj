/**
 * @file argparser.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of ArgParser class
 */

#include "argparser.hpp"
#include <vector>

ArgParser::ArgParser(): port{143},
                        mailbox{"INBOX"},
                        only_new{false},
                        only_headers{false},
                        secured{false}
{}

void ArgParser::parse(char *argv[], int argc) {
    // Convert array of const char* into a vector of strings for easier comparison
    std::vector<std::string> args(argv, argv + argc);

    for (auto it = args.begin(); it != args.end(); it++) {
        if (*it == "-a") {
            auth_file = *++it;
        }
        
        else if (*it == "-o") {
            out_dir = *++it;
        }

        else if (*it == "-p") {
            port = std::stoi(*++it);
        }

        else if (*it == "-n") {
            only_new = true;
        }

        else if (*it == "-h") {
            only_headers = true;
        }

        else if (*it == "-b") {
            mailbox = *++it;
        }

        else if (*it == "-T") {
            secured = true;

            // change the default port for TLS
            if (port == 143)
                port = 993;
        }

        else if (*it == "-c") {
            certfile = *++it;
        }

        else if (*it == "-C") {
            certaddr = *++it;
        }

        else server = *it;
    }

}