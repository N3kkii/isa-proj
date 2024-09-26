/**
 * @file argparser.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of ArgParser class
 */

#include "argparser.hpp"


ArgParser::ArgParser(): port{143},
                        mailbox{"INBOX"},
                        only_new{false},
                        only_headers{false},
                        secured{false}
{ /* empty constructor body */ }


void ArgParser::parse(char *argv[], int argc) {
    // Convert array of const char* into a vector of strings for easier comparison
    std::vector<std::string> args(argv, argv + argc);

    for (auto it = args.begin() + 1; it != args.end(); it++) {
        if (*it == "-a") {
            getOptionValue(args, it, this->auth_file);
        }
        
        else if (*it == "-o") {
            getOptionValue(args, it, this->out_dir);
        }

        else if (*it == "-p") {
            getOptionValue(args, it, this->port);
        }

        else if (*it == "-n") {
            only_new = true;
        }

        else if (*it == "-h") {
            only_headers = true;
        }

        else if (*it == "-b") {
            getOptionValue(args, it, this->mailbox);
        }

        else if (*it == "-T") {
            secured = true;

            // change the default port for TLS
            // TODO fix, if user wants encrypted, but also wants port 143
            if (port == 143)
                port = 993;
        }

        else if (*it == "-c") {
            getOptionValue(args, it, this->certfile);
        }

        else if (*it == "-C") {
            getOptionValue(args, it, this->certaddr);
        }

        else server = *it;
    }

}


void ArgParser::getOptionValue(const std::vector<std::string> &args, std::vector<std::string>::iterator &it, int &val) {
    if (std::next(it) != args.end()) {
        val = std::stoi(*++it);
    }
}

void ArgParser::getOptionValue(const std::vector<std::string> &args, std::vector<std::string>::iterator &it, std::string &val) {

    if (std::next(it) != args.end()) {
        val = *++it;
    }
}    