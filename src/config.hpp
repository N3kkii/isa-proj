/**
 * @file config.hpp
 * @author Vojtěch Adámek
 * 
 * @brief Config structure for passing arguments to IMAPClient
 */


#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

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

#endif
