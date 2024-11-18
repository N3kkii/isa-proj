/**
 * @file imapcl.cpp
 * @author Vojtěch Adámek
 * 
 * @brief Definition of IMAPClient class functionality
 */


#include "imapclient.hpp"
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
    state{State::DISCONNECTED},
    complete{false},
    uidvalidity{false},
    uidnext{"1"},
    buff{},
    bio{nullptr},
    ctx{nullptr},
    nmails{0}
{
    memset(this->buffer_in, 0, sizeof(this->buffer_in));
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
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
    if (this->synced) {
        std::cout << "All emails from server are already downloaded." << std::endl;
        return;
    }
    this->fetchMails();
}


void IMAPClient::connectToHost() {
    this->state = State::DISCONNECTED;

    std::string address = this->server + ":" + std::to_string(this->port);

    if (this->secured) { // use secured connection
        // create SSL context
        this->ctx = SSL_CTX_new(TLS_client_method());
        if (ctx == nullptr) {
            throw std::runtime_error("Error creating SSL context.");
        }

        // load certificates
        if (!this->certaddr.empty()) {
            if(!SSL_CTX_load_verify_locations(ctx, nullptr, this->certaddr.c_str())) {
                throw std::runtime_error("Cannot load certificates from folder.");
            }
        }

        else if (!this->certfile.empty()) {
            if(!SSL_CTX_load_verify_locations(ctx, this->certfile.c_str(), NULL)) {
                throw std::runtime_error("Cannot load certificate.");
            }
        }
        SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);


        SSL *ssl = SSL_new(ctx);
        // initialize BIO object for secured connection
        this->bio = BIO_new_ssl_connect(this->ctx);
        if(bio == nullptr) {
            throw std::runtime_error("Cannot initialize BIO object for connection.");
        }

        BIO_get_ssl(this->bio, ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
        BIO_set_conn_hostname(bio, address.c_str());

        if(BIO_do_connect(bio) <= 0) {
            throw std::runtime_error("Cannot estabilish secured connection.");
        }

        if(SSL_get_verify_result(ssl) != X509_V_OK) {
            throw std::runtime_error("Cannot verify the certificate.");
        }
        SSL_free(ssl);
    }

    else {
        this->bio = BIO_new_connect(address.c_str());

        if(bio == nullptr) {
            throw std::runtime_error("Cannot initialize BIO object for connection.");
        }

        if(BIO_do_connect(bio) <= 0) {
            throw std::runtime_error("Cannot connect to the server.");
        }
    }
    // Check for welcome message
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

    if(BIO_write(this->bio, outstr.c_str(), outstr.length()) < 0) {
        throw std::runtime_error("Failed to send a command");
    }

    this->checkResponse();
    
    // Increment tag for the next command
    this->tag++;
}

void IMAPClient::selectMailbox() {
    this->sendCommand("SELECT " + this->mailbox);
}

void IMAPClient::fetchMails() {
    std::string content{" (BODY[])"};
    if (this->only_headers) {
        content = " (BODY[HEADER])";
    }

    if (this->only_new){
        this->state = State::SEARCHING;
        this->sendCommand("UID SEARCH NEW");

        for (std::string& uid : this->newuids) {
            this->state = State::FETCHING;
            sendCommand("UID FETCH " + uid + content);
        }
        std::cout << "Downloaded " << this->nmails << " new mails." << std::endl;
        return;
    }

    this->state = State::FETCHING;
    if(!this->uidvalidity){
            this->sendCommand("UID FETCH 1:*" + content);
    }

    else {
        this->sendCommand("UID FETCH " + this->uidnext + ":*" + content); 
    }
}


void IMAPClient::checkResponse() {
    ssize_t nrecieved;
    while(!this->complete) {
        // try to get data from the server
        nrecieved = BIO_read(this->bio, this->buffer_in, BUFFER_SIZE);
        
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

void IMAPClient::checkTagged(const std::string response) {
    int code = -1;
    if (response.starts_with("A" + std::to_string(this->tag) + " OK")) {
        code = 0;
    }

    else if (response.starts_with("A" + std::to_string(this->tag) + " NO")) {
        code = 1;
    }

    else if (response.starts_with("A" + std::to_string(this->tag) + " BAD")) {
        throw std::runtime_error("Internal error.");
    }

    else return;

    switch (this->state) {
        case State::CONNECTED:
            if (!code) this->state = State::LOGGED;
            else if (code) throw std::runtime_error("Invalid credentials.");
            break;

        case State::LOGGED:
            if (!code) this->state = State::SELECTED;
            else if (code) throw std::runtime_error("Desired mailbox does not exist.");
            break;

        case State::FETCHING:
            if (!code) {
                this->state = State::SELECTED;
                if (!this->only_new) {
                    if (this->only_headers) {
                        std::cout << "Downloaded " << this->nmails << " email headers." << std::endl;

                    }
                    else {
                        std::cout << "Downloaded " << this->nmails << " emails." << std::endl;
                    }
                }
            }
            else if (code) throw std::runtime_error("Could not fetch data from the server.");
            break;

        case State::SEARCHING:
            if (!code) this->state = State::SELECTED;
            else if (code) throw std::runtime_error("Could not search for new mails.");
            break;
        
        default:
            break;
    }
    this->complete = true;
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
                return;
            }
        }

        // Selecting mailbox
        else if (this->state == State::LOGGED) {
            
            // handle untagged responses
            if (response.starts_with("* OK") && !this->only_new) {
                std::string argline = response.substr(response.find("[")+1, response.find("]") - response.find("[")-1);
                std::istringstream iss{argline};
                std::string arg;
                iss >> arg;

                if (arg == "UIDVALIDITY" && !this->only_headers) {
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

                    else {
                        std::ifstream uidnext_f(filename);
                        uidnext_f >> this->uidnext;
                        std::string new_uidnext;
                        iss >> new_uidnext;
                        if(new_uidnext == this->uidnext) {
                            this->synced = true;
                        }
                    }
                }
            }
        }

         else if (this->state == State::SEARCHING) {
            if (response.starts_with("* SEARCH")) {
                std::istringstream iss(response.substr(8));
                std::string uid;

                 while (iss >> uid) {
                    this->newuids.push_back(uid);
                }
            }
        }

        else if (this->state == State::FETCHING) {
            static unsigned long nbytes = 0;
            static std::string filename;
            static std::string uid;
            if (getting_data){

                if (this->buff.length() < nbytes) { 
                    return; // Dont have enough data, return to checkResponse() (for readability)
                }

                else {
                    std::string data = this->buff.substr(0, nbytes);
                    this->buff.erase(0, nbytes + 3); // nbytes + 3 to remove ')\r\n' also
                    std::ofstream mailfile(filename);
                    mailfile << data;
                    nmails++;

                    // Change UIDNEXT only when downloading complete emails
                    if(!this->only_headers && !this->only_new) {
                        std::ofstream uidnext_f(this->out_dir + "/.uidnext");
                        uidnext_f << std::to_string(std::stoi(uid) + 1);
                    }
                    
                    getting_data = false;
                }
            }

            else {
                if (response.starts_with("*")) {
                nbytes = stoi(response.substr(response.find("{")+1, response.find("}") - response.find("{")-1));
                
                std::istringstream iss{response.substr(response.find("UID"), response.length()-1)}; // MOVE into ELSE
                iss >> uid >> uid;
                filename = this->out_dir + "/" + uid + "." + this->mailbox + "." + this->server;

                getting_data = true;
                }
            }
        }
        checkTagged(response);
    }
}


void IMAPClient::cleanup() {
    if (this->state != State::DISCONNECTED) {
        this->logout();
    }

    if (this->bio != nullptr) {
        BIO_free_all(this->bio);
    }

    if (this->ctx != nullptr) {
        SSL_CTX_free(this->ctx);
    }
}
