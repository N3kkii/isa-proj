#ifndef ARGS_HPP
#define ARGS_HPP

/**
 * @file argparser.hpp
 * @author Vojtěch Adámek
 * 
 * @brief Header file for ArgParser class
 * 
 * Class used for parsing command line arguments.
 * 
 * Parameters:
 *      Mandatory:
 *          server
 *          -a auth_file
 *          -o out_dir
 * 
 *      Optional:
 *          -p port             Specifies server port number, defaults to 143 (993 with TLS)
 *          -n                  Read only new messages
 *          -h                  Download only mail headers
 *          -b MAILBOX          Specifies the mailbox, defaults to INBOX
 *          -T                  Use secured communication
 *              -c certfile     Specifies file with certificates for verifying the server ceritficate
 *              -C certaddr     Specifies folder with certificates
 *
 */

#include <string>


class ArgParser {
public:
    // Mandatory parameters
    std::string server;
    std::string auth_file;
    std::string out_dir;
    
    // Optional parameters
    int port;
    std::string mailbox;
    std::string certfile;
    std::string certaddr;
    bool only_new;
    bool only_headers;
    bool secured;


    /**
     * @brief Constructs ArgParser object
     */
    ArgParser();


    /**
     * @brief Default destructor
     */
    ~ArgParser() = default;


    /**
    * @brief Parses the command line parameters
    * 
    * @param argv from main
    * @param argc from main
    * 
    * @todo fix missing paremeter argument
    */
    void parse(char *argv[], int argc);

private:

};

#endif
