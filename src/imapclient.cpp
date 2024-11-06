/**
 * @file imapcl.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of IMAPClient class functionality
 */


#include "imapclient.hpp"
#include <fstream>

IMAPClient::IMAPClient(std::string &server, std::string &auth_file, std::string &out_dir, int port, std::string mailbox, 
                        std::string certfile, std::string certaddr, bool only_new, bool only_headers, bool secured):
    server{server},
    auth_file{auth_file},
    out_dir{out_dir},
    port{port},
    mailbox{mailbox},
    certfile{certfile},
    certaddr{certaddr},
    only_new{only_new},
    only_headers{only_headers},
    secured{secured},
    tag{1},
    logged{false}
{
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
    current_command = CommandType::NONE;
    this->res = nullptr;
    this->sockfd = -1;
};


IMAPClient::IMAPClient(struct Config &config) :
    server{config.server},
    auth_file{config.auth_file},
    out_dir{config.out_dir},
    port{config.port},
    mailbox{config.mailbox},
    certfile{config.certfile},
    certaddr{config.certaddr},
    only_new{config.only_new},
    only_headers{config.only_headers},
    secured{config.secured},
    tag{1},
    logged{false}
{ 
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
    current_command = CommandType::NONE;
    this->res = nullptr;
    this->sockfd = -1;
}


IMAPClient::~IMAPClient() {
    cleanup();
}


void IMAPClient::start() {
    std::cout << "Connectim" << std::endl;
    this->connectToHost();
    std::cout << "Jdu na prihlasovani" << std::endl;
    this->login();
    this->selectMailbox();
    this->current_command = CommandType::FETCH;
    this->sendCommand("FETCH 1:5 (BODY[HEADER])");
}


void IMAPClient::connectToHost() {

    // Initizalizing structures for getaddrinfo
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; 

    // Resolve domain name with getaddrinfo
    if (getaddrinfo(this->server.c_str(), std::to_string(this->port).c_str(), &hints, &this->res) != 0) {
        throw std::runtime_error("Error translating address");
    }

    // Create a communication socket
    this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Error creating socket.");
    }

    for (auto addr = this->res; addr != nullptr; addr = addr->ai_next) {
        // Try to connect, if successful, exit the loop
        // TODO Set the socket to nonblocking and set a timeout to avoid endless retransmission
        if (connect(this->sockfd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }

        // If addr is the last address, throw std::runtime_error
        if (addr->ai_next == nullptr) {
            throw std::runtime_error("Cannot connect to hostname");
        }
    }
    // TODO Check response (welcome message)
    this->current_command = CommandType::CONNECT;
    std::cout << "Jdu checknout response po connectnuti" << std::endl;
    this->checkResponse();
    std::cout << "Jsem pripojen" << std::endl;
}


void IMAPClient::login() {
    std::ifstream file(this->auth_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open auth file.");
    }

    std::string username, password;
    
    // Get username and password from auth file
    if ((std::getline(file, username) && std::getline(file, password)) == false) {
        file.close();
        throw std::runtime_error("Wrong auth file format.");
    }

    file.close();
    this->current_command = CommandType::LOGIN;
    this->sendCommand("LOGIN " + username + " " + password);
    this->logged = true;
}


void IMAPClient::logout() {
    this->current_command = CommandType::LOGOUT;
    this->sendCommand("LOGOUT");
}

void IMAPClient::sendCommand(const std::string &cmd) {
    // Construct an outgoing command
    std::string outstr = "A" + std::to_string(this->tag) + " " + cmd + " \r\n";
    if(send(this->sockfd, outstr.c_str(), outstr.length(), 0) < 0) {
        throw std::runtime_error("Failed to send a command");
    }
    this->checkResponse(true);
    
    // Increment tag for the next command
    this->tag++;
}

void IMAPClient::selectMailbox() {
    this->current_command = CommandType::SELECT;
    this->sendCommand("SELECT " + this->mailbox);
}


void IMAPClient::checkResponse(bool tagged) {
    std::string buff;
    ssize_t nrecieved;
    bool complete = false;

    // call recv as long as we don't get /r/n sequence
    // fix this, loop until I get tagged response
    if (tagged) {
        while(!complete) {
            nrecieved = recv(this->sockfd, this->buffer_in, BUFFER_SIZE, 0);
            
            if(nrecieved == -1 || nrecieved == 0){
                throw std::runtime_error("Server closed the connection.");
            }
            buff.append(this->buffer_in, nrecieved);
            if(buff.find("A" + std::to_string(this->tag) + " OK") != std::string::npos) {
                complete = true;
            }
            this->processResponse(buff);
        }

    }
    

    else {
        while(!complete) {
            nrecieved = recv(this->sockfd, this->buffer_in, BUFFER_SIZE, 0);
            
            if(nrecieved == -1 || nrecieved == 0){
                throw std::runtime_error("Server closed the connection.");
            }

            buff.append(this->buffer_in, nrecieved);
            if (buff.ends_with("\r\n")) {
                complete = true;
            }
            this->processResponse(buff);
        }
    }
    
}

void IMAPClient::processResponse(std::string &buff) {
    std::size_t idx = 0;
    std::string response;
    int i = 0;

    while (!buff.empty()){
        idx = buff.find("\r\n");
        response = buff.substr(0, idx + 2);
        buff = buff.erase(0, idx + 2);
        std::string filename = this->out_dir + std::to_string(i);

        if (this->current_command == CommandType::SELECT) {

        }

        else if (this->current_command == CommandType::FETCH) {
            //std::ofstream file(filename);
            //file << response;
        }
        std::cout << response;
        // switch (this->current_command)
        // {
        
        // // case CommandType::LOGOUT:
        // //     if (response.starts_with("A" + std::to_string(this->tag) + " OK")) {
        // //         std::cout << "Positive tagged response" << std::endl;
        // //     }

        // //     else if (response.starts_with("* BYE")) {
        // //         std::cout << "Positive untagged response" << std::endl;
        // //     }

        // //     else {
        // //         std::cout << "NON-Positive response" << std::endl;
        // //     }
        // //     break;

        // case CommandType::SELECT:
        //     break;
        
        // // case CommandType::FETCH:
        // //     std::ofstream file(filename);
        // //     break;

        // default:
        //     std::cout << response;
        //     break;
        // 
        i++;
    }
}


void IMAPClient::cleanup() {
    if (this->logged) {
        this->logout();
    }

    if (this->sockfd != -1){
        close(this->sockfd);
    }

    if (this->res != nullptr) {
     freeaddrinfo(res);
    }
}
