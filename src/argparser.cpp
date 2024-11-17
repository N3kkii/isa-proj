/**
 * @file argparser.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of ArgParser class
 */

#include "argparser.hpp"

ArgParser::ArgParser(): server{""},
                        auth_file{""},
                        out_dir{""},
                        port{143},
                        mailbox{"INBOX"},
                        certfile{""},
                        certaddr{""},
                        only_new{false},
                        only_headers{false},
                        secured{false},
                        display_help{false}
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
            // TODO fix, if user wants encrypted, but also wants port 143 for some reason
            if (port == 143)
                port = 993;
        }

        else if (*it == "-c") {
            getOptionValue(args, it, this->certfile);
        }

        else if (*it == "-C") {
            getOptionValue(args, it, this->certaddr);
        }

        else if (*it == "--help") {
            this->printHelp();
            this->display_help = true;
        }

        else server = *it;
    }
}


void ArgParser::getOptionValue(const std::vector<std::string> &args, std::vector<std::string>::iterator &it, int &val) {
    if (std::next(it) != args.end()) {
        try {
            val = std::stoi(*++it);
        }
        catch(std::invalid_argument&) {
            throw std::invalid_argument("port must be a number");
        }
    }
}


void ArgParser::getOptionValue(const std::vector<std::string> &args, std::vector<std::string>::iterator &it, std::string &val) {

    if (std::next(it) != args.end()) {
        val = *++it;
    }
}

void ArgParser::printHelp() {
    std::cout << 
R"(Usage: imapcl server -a auth_file -o out_dir [OPTIONS]

 OPTIONS:
    -p port         Specifies server port number, defaults to 143 (993 with TLS)
    -n              Read only new messages
    -h              Download only mail headers
    -b MAILBOX      Specifies the mailbox, defaults to INBOX
    -T              Use secured communication
    -c certfile     Specifies file with certificates for verifying the server ceritficate
    -C certaddr     Specifies folder with certificates
    --help          Shows this help)"
    << std::endl;
}

Config ArgParser::getConfig() {
    Config config;

    config.server = this->server;
    config.auth_file = this->auth_file;
    config.out_dir = this->out_dir;
    config.port = this->port;
    config.mailbox = this->mailbox;
    config.certfile = this->certfile;
    config.certaddr = this->certaddr;
    config.only_new = this->only_new;
    config.only_headers = this->only_headers;
    config.secured = this->secured;

    return config;   
}

void ArgParser::check() {
    if (this->server == "" || this->out_dir == "" || this->auth_file == "") {
        throw std::invalid_argument("Mandatory arguments not provided.");
    }

    if (this->certaddr.empty() && this->certfile.empty() && this->secured) {
        std::cerr << "Warning: -c and -C flags without -T, ignoring them" << std::endl;
    }
}
