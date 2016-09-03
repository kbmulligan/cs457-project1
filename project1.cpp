// project1.cpp - A chat server and client that pass messages back and forth across a socket connection
// Author: K. Brett Mulligan
// Date: Sep 2016
// CSU - Comp Sci
// CS457 - Networks
// Dr. Indrajit Ray
// License: MIT

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;

const int BACKLOG = 1;

void usage (int argc, char* argv[]);
int server_mode (void);
int client_mode (void);
int check_args (void);
int start_listening (void);
int print_status (int ip, int port);
int get_ip (void);

char* packetize (string msg);




int main (int argc, char* argv[]) {

    cout << "Hello networking world!" << endl;
    cout << "Successful makefile?" << endl;

    if (argc == 1) {
        server_mode();
    } else if (argc == 5) {
        client_mode();
    } else {
        usage(argc, argv);
    }

    return 0;
}

int get_ip () {
 
    int ip = 0;
    string port = "31339";

    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    string host = "localhost";

    char hostname[64];

    gethostname(hostname, 64);
    
    status = getaddrinfo(hostname,  NULL, &hints, &res); 
    if (status != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return 2;
    }

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;
        

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
        cout << ipver << " : " << ipstr << endl;

    }

    freeaddrinfo(res);

    return ip;
}

int server_mode () {

    cout << "Server mode..." << endl;

    get_ip();
    print_status(0, 0);
    start_listening();

    return 0;
}

int client_mode () {

    cout << "Client mode..." << endl;


    return 0;
}


int check_args () {
    cout << "Checking arguments..." << endl;


    return false;
}


int print_status (int ip, int port) {

    cout << "Status..." << endl;

    cout << "IP :   " << ip << endl;
    cout << "PORT : " << port << endl;

    return 0;
}

int start_listening () {
    
    string port = "31337"; 
    struct addrinfo hints, *res;
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
    
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (status == -1) {
        cerr << "start_listening error: socket" << endl;
        return 2;
    }

    status = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {

        cerr << "start_listening error: bind" << endl;
        return 2;
    }
    
    status = listen(sockfd, BACKLOG);
    if (status == -1) {
        cerr << "start_listening error: listen" << endl;
        return 2;
    }

    struct sockaddr_storage peeraddr;
    socklen_t peeraddrsize = sizeof(peeraddr);

    connectedfd = accept(sockfd, (struct sockaddr *)&peeraddr, &peeraddrsize);

    return 0;
}

// Stores a pointer to a memory block formatted for packet transmission in pointer 'data'
// Returns 0 on success
int packetize (string msg, char* data) {
    short version = 457;
    short msg_length = -1;

    return 0;
}


void usage (int argc, char* argv[]) {
    cout << "Usage: " << argv[0] << "[-p PORT -s SERVER]" << endl;
    return;
}

