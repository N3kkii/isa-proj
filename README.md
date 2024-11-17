# IMAP client with TLS support
CLI client capable of downloading mail from a specified IMAP server. Supports TLS.
## Compilation
### Requirements:
* make
* g++10 or newer
* OpenSSL library 3.0 or newer

Compile the program using ```make```, for debugging use ```make debug```.

## Usage

```imapcl server -a auth_file -o out_dir [-T] [-c] [-C] [-p port] [-b mailbox] [-n] [-h]```

## Authors:
Vojtěch Adámek, VUT FIT 2024