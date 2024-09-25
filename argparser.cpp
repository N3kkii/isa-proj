/**
 * @file argparser.cpp
 * @author Vojtěch Adámek
 * @brief Definition of ArgParser class
 */

#include "argparser.hpp"
#include <vector>

void ArgParser::parse(char *argv[], int argc) {
    // Convert C style array of const char* into a vector of strings
    std::vector<std::string> args(argv, argv + argc);

    for (auto it = args.begin(); it != args.end(); it++) {
        if (*it == "-a") {

        }
        
        else if (*it == "-o") {

        }

        else if (*it == "-p") {

        }

        else if (*it == "-n") {

        }

        else if (*it == "-h") {

        }

        else if (*it == "-b") {

        }

        else if (*it == "-T") {

        }

        else if (*it == "-c") {

        }

        else if (*it == "-C") {

        }
    }

}