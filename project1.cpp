// project1.cpp - A chat server and client that passes messages 
//                back and forth across a socket connection
// Author: K. Brett Mulligan
// Date: Sep 2016
// CSU - Comp Sci
// CS457 - Networks
// Dr. Indrajit Ray
// License: MIT
// References : Beej's Guide to Socket Programming
//              https://beej.us/guide/bgnet/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

const int PORT = 55333;
const int BACKLOG = 1;
const unsigned int CHARLIMIT = 140;
const unsigned int HEADERSIZE = 4;

void usage (int argc, char* argv[]);
int server_mode (void);
int client_mode (string ip, string port);
bool check_args (string serverval, string portval);
bool is_valid_port (string p);
bool is_valid_ip_address (string s);
int start_listening (int portreq);
int print_status (string ip, string port);
int make_connection (string ip, string port);
string get_ip (void);
int comm_loop(int socketfd);

int recv_msg(int socketfd);
int send_msg(int socketfd);
string prompt_for_msg();
bool check_msg (const char* msg);


int packetize (string msg, char* data);


int main (int argc, char* argv[]) {

    cout << "Welcome to chat!" << endl;

    int opt = 0;
    string portval, serverval;

    while ((opt = getopt(argc, argv, "p:s:")) != -1) {
        switch (opt) {
            case 'p':
                portval = optarg;
                break;
            case 's':
                serverval = optarg;
                break;
            case '?':
                if (optopt == 's' || optopt == 'p') {
                    cerr << "Option -" << optopt << " requires an argument." << endl;
                } else {
                    cerr << "Unknown option " << optopt << endl;
                }

            default:
                cerr << "getopt error: default ... aborting!" << endl;
                abort();
        }

    }

    if (argc == 1) {
        server_mode();
    } else if (serverval.length() and portval.length()) {
        if (check_args(serverval, portval)) {
            cout << "Invalid arguments...exiting" << endl;
            usage(argc, argv);
            exit(1);
        } else {
            cout << "Valid!" << endl;
        }

        client_mode(serverval, portval);
    } else {
        usage(argc, argv);
    }

    return 0;
}

string get_ip () {
 
    string ip;

    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    char hostname[64];

    gethostname(hostname, 64);
    
    //cout << "Getting hostname : " << hostname << endl;
    status = getaddrinfo(hostname,  NULL, &hints, &res); 
    if (status != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return string("ERROR: check cerr");
    }

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        string ipver;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = string("IPv4");
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = string("IPv6");
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        //cout << ipver << " : " << ipstr << endl;

    }

    freeaddrinfo(res);

    ip = string(ipstr);
    return ip;
}

int get_ip_from_addr (struct addrinfo* addr, char* ipstr) {
 
    void *address;
    short sin_fam;

    if (addr->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr->ai_addr;
        address = &(ipv4->sin_addr);
        sin_fam = ipv4->sin_family;
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addr->ai_addr;
        address = &(ipv6->sin6_addr);
    }

    inet_ntop(sin_fam, address, ipstr, sizeof(ipstr));

    return 0;
}

int server_mode () {

    cout << "Server mode..." << endl;
    start_listening(PORT);

    return 0;
}

int client_mode (string ip, string port) {

    cout << "Client mode..." << endl;
    make_connection(ip, port);

    return 0;
}

int make_connection (string ip, string port) {

    cout << "Connecting to..." << endl;
    cout << "SERVER : " << ip << endl;
    cout << "PORT   : " << port << endl;

    struct addrinfo hints, *res;
    int sockfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(ip.c_str(), port.c_str(), &hints, &res);
    if (status == -1) {
        cerr << "make_connection error: getaddrinfo" << endl;
        return 2;
    }
    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (status == -1) {
        cerr << "make_connection error: socket" << endl;
        return 2;
    }

    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        close(sockfd);
        cerr << "make_connection error: connect" << endl;
        return 2;
    }

    // done with this addrinfo
    freeaddrinfo(res);
    
    // check if everything's good to go, then start comm
    if (!sockfd) {
        cerr << "make_connection error: sockfd == NULL" << endl;
    }

    send_msg(sockfd);
    comm_loop(sockfd);    
    
    close(sockfd);
    return 0;
}

bool check_args (string serverval, string portval) {
    cout << "Checking arguments... ";

    bool invalid = false;

    if (is_valid_port(portval)) {
        invalid = true;
        cerr << "INVALID port!!!" << endl;
    }
    
    if (is_valid_ip_address(serverval)) {
        invalid = true;
        cerr << "INVALID server!" << endl;
    }

    //cout << "Arguments valid!" << endl;
    return invalid;
}

bool is_valid_port (string p) {
    bool invalid = false;
    char* end;
    end = NULL;

    if (strtol(p.c_str(), &end, 0) == 0 || *end != '\0') {
        invalid = true;
    }

    return invalid;
} 

bool is_valid_ip_address (string s) {
    bool invalid = false;
    struct sockaddr_in socketaddress;

    if (inet_pton(AF_INET, s.c_str(), &(socketaddress.sin_addr)) == 0) {
        invalid = true;
    }

    return invalid;
} 

int print_status (string ip, string port) {

    cout << "Status..." << endl;

    cout << "IP   :   " << ip << endl;
    cout << "PORT :   " << port << endl;

    return 0;
}

int start_listening (int portreq) {
    
    string port(to_string(portreq)); 
    struct addrinfo hints, *res, *p;
    int sockfd, connectedfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;       // use the IP of this machine

    int status = getaddrinfo(NULL, port.c_str(), &hints, &res);
    if (status == -1) {
        cerr << "start_listening error: getaddrinfo" << endl;
        return 2;
    }

    
    int usable_addresses = 0;
    for (p = res; p != NULL; p = p->ai_next) {
        usable_addresses++;
        
        //cout << "Address info : ";
        char ipaddrstr[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, &((struct sockaddr_in *)(res->ai_addr))->sin_addr, ipaddrstr, INET6_ADDRSTRLEN); 
        //cout << ipaddrstr << endl;
    } 
    //cout << "getaddrinfo usable addresses : " << usable_addresses << endl;

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (status == -1) {
        cerr << "start_listening error: socket" << endl;
        return 2;
    }

    status = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        close(sockfd);
        cerr << "start_listening error: bind" << endl;
        return 2;
    }
    
    status = listen(sockfd, BACKLOG);
    if (status == -1) {
        close(sockfd);
        cerr << "start_listening error: listen" << endl;
        return 2;
    }
 
    cout << "Waiting for connection to " << get_ip() << " on port " << port << endl;

    struct sockaddr_storage peeraddr;
    socklen_t peeraddrsize = sizeof(peeraddr);

    connectedfd = accept(sockfd, (struct sockaddr *)&peeraddr, &peeraddrsize);
    if (status == -1) {
        cerr << "start_listening error: accept" << endl;
        return 2;
    } else {
        char peeraddrstr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET, &((struct sockaddr_in*) &peeraddr)->sin_addr, peeraddrstr, INET6_ADDRSTRLEN);
        cout << "Good connection!" << endl;
        cout << "Connection from : " << peeraddrstr << endl;
        comm_loop(connectedfd);
    }
    
    freeaddrinfo(res);

    close(sockfd);
    close(connectedfd);
    return 0;
}

int comm_loop (int socketfd) {
    
    while (1) {
        recv_msg(socketfd);
        send_msg(socketfd); 
    }

    return 0;
}

string prompt_for_msg (void) {
    string msg;

    cout << "Type message: ";
    getline(cin, msg);

    return msg; 
}

int send_msg (int socketfd) {
    
    string msg;
    int msglen = sizeof(msg);
    char* data = NULL;

    cout << "Type message: ";
    getline(cin, msg);

    while (check_msg(msg.c_str())) {
        cout << "Type message: ";
        getline(cin, msg);
    }

    msglen = msg.length();

    int required = HEADERSIZE + sizeof(msg);
    data = (char *)malloc(required);
    memset(data, 0, required);

    packetize(msg, data);

    int flags = 0;

    int bytes_sent = send(socketfd, data, required, flags);

    if (bytes_sent < msglen) {
        cout << "ERROR: Only partial message sent." << endl;
    }
 
    free(data);
 
    return 0;
}

int recv_msg (int socketfd) {

    int bufferlen = 255;
    char buffer[bufferlen];
    int flags = 0;
 
    cout << "Waiting for message..." << endl;
    int bytes_received = recv(socketfd, buffer, bufferlen, flags);

    unsigned short version = ntohs(buffer[0]);
    unsigned short msglen = ntohs(buffer[2]);
    cout << "Version: " << version << endl;
    cout << "Message length: " << msglen << endl;
    cout << "Message received: " << buffer + 4 << " (" << bytes_received << " bytes)" << endl;

    return 0;
}

// report error if msg is greater than 140 chars
bool check_msg (const char* msg) {

    bool is_too_long = false;
    if (strlen(msg) > CHARLIMIT) {
        cout << "ERROR: Input too long!" << endl;
        is_too_long = true;
    }

    return is_too_long;
}

// Stores a pointer to a memory block formatted for packet transmission in pointer 'data'
// Returns 0 on success
int packetize (string msg, char* data) {
    short version = 457;
    short msg_length = 0;
    const char* message = msg.c_str();

    unsigned short version_net = htons(version);
    unsigned short msg_length_net = htons(msg.length());

    memcpy(data, &version_net, sizeof(version_net));
    memcpy(data + sizeof(version), &msg_length_net, sizeof(msg_length_net));
    memcpy(data + sizeof(version) + sizeof(msg_length), message, sizeof(message));
 
    //stringstream packet;
    //packet << htons(version);
    //packet << htons(msg_length);
 
    //cout << packet << endl;

    return 0;
}

void usage (int argc, char* argv[]) {
    cout << "Usage: " << argv[0] << " [-p PORT -s SERVER]" << endl;
    return;
}

