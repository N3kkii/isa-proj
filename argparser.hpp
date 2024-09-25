#ifndef ARGS_HPP
#define ARGS_HPP

/**
 * @file argparser.hpp
 * @author Vojtěch Adámek
 * @brief Header file for ArgParser class
 * 
 * Class used for parsing command line arguments.
 * 
 * Parameters:
 *      Mandatory:
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
    ArgParser() = default;
    ~ArgParser() = default;

    void parse(char *argv[], int argc);

};

#endif
