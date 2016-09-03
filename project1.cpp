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

int server_mode (void);
int client_mode (void);
int check_args (void);
int start_listening (void);
int print_status (int ip, int port);
int get_ip (void);

using namespace std;

int main () {

    cout << "Hello networking world!" << endl;
    cout << "Successful makefile?" << endl;

    return 0;
}

int get_ip () {
 
    int ip = 0;

    struct addrinfo hints, *res, *p;
    int status;
    char ipstring[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    string host = "localhost";
    
    status = getaddrinfo(host.c_str(), NULL, &hints, &res); 
    if (status != 0) {
        cerr << "getaddrinfo: " << gai_strerror(status) << endl;
        return 2;
    }


    return ip;
}

int server_mode () {

    cout << "Server mode..." << endl;

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



    return 0;
}



