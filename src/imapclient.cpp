/**
 * @file imapcl.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of IMAPClient class functionality
 */


#include "imapclient.hpp"
#include <config.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>


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
    res{nullptr},
    state{State::DISCONNECTED},
    complete{false},
    uidvalidity{false},
    buff{}
{
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
}


IMAPClient::IMAPClient(struct Config &config) :
    IMAPClient(config.server, config.auth_file, config.out_dir, config.port, 
               config.mailbox, config.certfile, config.certaddr, 
               config.only_new, config.only_headers, config.secured)
{ /* empty body */ }


IMAPClient::~IMAPClient() {
    cleanup();
}


void IMAPClient::start() {
    this->connectToHost();
    this->login();
    this->selectMailbox();
    this->fetchMails();
}


void IMAPClient::connectToHost() {

    this->state = State::DISCONNECTED;

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
    this->checkResponse();
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
    this->sendCommand("LOGIN " + username + " " + password);
}


void IMAPClient::logout() {
    this->state = State::LOGOUT;
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
    this->sendCommand("SELECT " + this->mailbox);
}

void IMAPClient::fetchMails() {
    this->state = State::FETCHING;
    if(!this->uidvalidity){
        this->sendCommand("UID FETCH 1:* (BODY[])");
    }
    else {
        this->sendCommand("UID FETCH 1:* (BODY[])");
    }
}


void IMAPClient::checkResponse(bool tagged) {
    ssize_t nrecieved;
    while(!this->complete) {
        // try to get data from the server
        nrecieved = recv(this->sockfd, this->buffer_in, BUFFER_SIZE, 0);
        
        if(nrecieved == -1 || nrecieved == 0){
            throw std::runtime_error("Server closed the connection.");
            this->state = State::DISCONNECTED;
        }
        buff.append(this->buffer_in, nrecieved);
        
        // process recieved data
        this->processResponse();
    }
    this->complete = false;
}

void IMAPClient::processResponse() {
    std::size_t idx = 0;
    std::string response;
    static bool getting_data = false;

    while (!this->buff.empty() && !this->complete){
        if (!getting_data) {
            idx = buff.find("\r\n");

            if (idx == std::string::npos) {
                return;
            }

            response = this->buff.substr(0, idx + 2);
            this->buff = this->buff.erase(0, idx + 2);
        }
        

        // Recieve welcome message
        if (this->state == State::DISCONNECTED) {
            if (response.starts_with("* OK")) {
                this->complete = true;
                this->state = State::CONNECTED;
            }
        }

        // Logging in
        else if (this->state == State::CONNECTED) {
            if (response.starts_with("A" + std::to_string(this->tag))) {
                this->complete = true;
                this->state = State::LOGGED;
            }
        }

        // Selecting mailbox
        else if (this->state == State::LOGGED) {
            
            // handle untagged responses
            if (response.starts_with("* OK")) {
                std::string argline = response.substr(response.find("[")+1, response.find("]") - response.find("[")-1);
                std::istringstream iss{argline};
                std::string arg;
                iss >> arg;

                if (arg == "UIDVALIDITY") {
                    std::string old_uid, new_uid;
                    iss >> new_uid;
                    std::string filename = this->out_dir + "/.uidvalidity";

                    // if .uidvalidity exists, try to read it
                    if (std::filesystem::exists(filename)) {
                        
                        // check existing UIDVALIDITY
                        std::ifstream uidvalid_f(filename);
                        if (uidvalid_f.is_open()) {
                            std::getline(uidvalid_f, old_uid); 

                            // if UIDVALIDITY differs, reopen the file in write mode and rewrite
                            if (old_uid != new_uid) {
                                std::ofstream uidvalid_f(filename);
                                uidvalid_f << new_uid;
                            }
                            else this->uidvalidity = true;
                        }
                        else {
                            throw (std::runtime_error("Cannot open .uidvalidity file."));
                        }
                    }

                    // else create .uidvalidity file
                    else {
                        std::ofstream uidvalid_f(filename);
                        if (uidvalid_f.is_open()) {
                            uidvalid_f << new_uid;
                        }
                        
                        else {
                            throw (std::runtime_error("Cannot create .uidvalidity file."));
                        }
                    }
                }

                else if (arg == "UIDNEXT") {
                    std::string filename = this->out_dir + "/.uidnext";

                    if (!std::filesystem::exists(filename)) {
                        std::ofstream uidnext_f(filename);
                        if (uidnext_f.is_open()) {
                            uidnext_f << "1";
                        }
                        
                        else {
                            throw (std::runtime_error("Cannot create .uidnext file."));
                        }
                    }
                }
            }

            else if (response.starts_with("A" + std::to_string(this->tag))) {
                this->complete = true;
                this->state = State::SELECTED;
            }
        }

        else if (this->state == State::FETCHING) {
            static unsigned long nbytes = 0;
            static std::string filename;
            if (getting_data){

                if (this->buff.length() < nbytes) { 
                    return; // Dont have enough data, return to checkResponse() (for readability)
                }

                else {
                    std::string data = this->buff.substr(0, nbytes);
                    this->buff.erase(0, nbytes + 3); // nbytes + 3 to remove ')\r\n' also
                    std::ofstream mailfile(filename);
                    mailfile << data;
                    getting_data = false;
                }
            }

            else {
                if (response.starts_with("*")) {
                nbytes = stoi(response.substr(response.find("{")+1, response.find("}") - response.find("{")-1));
                
                std::istringstream iss{response.substr(response.find("UID"), response.length()-1)}; // MOVE into ELSE
                std::string uid;
                iss >> uid >> uid;
                filename = this->out_dir + "/" + uid + "." + this->mailbox + "." + this->server;

                getting_data = true;
                std::cout << response << std::endl;
                }

                else if(response.starts_with("A" + std::to_string(this->tag))) {
                    this->complete = true;
                    this->state = State::SELECTED;
                }
            }
        }

        else if (this->state == State::SELECTED || this->state == State::LOGOUT) {
            if(response.starts_with("A" + std::to_string(this->tag))) {
                this->complete = true;
            }
        }
    }
}


void IMAPClient::cleanup() {
    if (this->state != State::DISCONNECTED) {
        this->logout();
    }

    if (this->sockfd != -1){
        close(this->sockfd);
    }

    if (this->res != nullptr) {
     freeaddrinfo(res);
    }
}
