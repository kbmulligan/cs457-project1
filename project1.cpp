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

const int PORT = 31337;
const int BACKLOG = 1;

void usage (int argc, char* argv[]);
int server_mode (void);
int client_mode (string ip, string port);
int check_args (void);
int start_listening (int portreq);
int print_status (string ip, string port);
int make_connection (string ip, string port);
int get_ip (void);
int comm_loop(int socketfd);

int recv_msg(int socketfd);
int send_msg(int socketfd);
string prompt_for_msg();


char* packetize (string msg);




int main (int argc, char* argv[]) {

    cout << "Welcome to chat!" << endl;

    if (argc == 1) {
        server_mode();
    } else if (argc == 5) {
        string ip(argv[4]);
        string port(argv[2]);
        client_mode(ip, port);
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
        cout << ipver << " : " << ipstr << endl;

    }

    freeaddrinfo(res);

    return ip;
}

int get_ip_from_addr (struct addrinfo* addr, char* ipstr) {
 
    void *address;

    if (addr->ai_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr->ai_addr;
        address = &(ipv4->sin_addr);
    } else {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addr->ai_addr;
        address = &(ipv6->sin6_addr);
    }

    inet_ntop(addr->ai_family, address, ipstr, sizeof(ipstr));

    return 0;
}

int server_mode () {

    cout << "Server mode..." << endl;

    //get_ip();
    print_status(" NOTHING ", " NOTHING ");
    start_listening(PORT);

    return 0;
}

int client_mode (string ip, string port) {

    cout << "Client mode..." << endl;

    cout << "SERVER : " << ip << endl;
    cout << "PORT   : " << port << endl;

    make_connection(ip, port);

    return 0;
}

int make_connection (string ip, string port) {

    struct addrinfo hints, *res;
    int sockfd, connectedfd = -1;

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

    
    // check if everything's good to go, then start comm
    send_msg(sockfd);
    comm_loop(sockfd);    
    
    close(sockfd);
    return 0;
}

int check_args () {
    cout << "Checking arguments..." << endl;



    return false;
}


int print_status (string ip, string port) {

    cout << "Status..." << endl;

    cout << "IP   :   " << ip << endl;
    cout << "PORT :   " << port << endl;

    return 0;
}

int start_listening (int portreq) {
    
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
 
    char ipstr[INET6_ADDRSTRLEN];
    get_ip_from_addr(res, ipstr);
    print_status(ipstr, port);
    cout << "Waiting for connection..." << endl;

    struct sockaddr_storage peeraddr;
    socklen_t peeraddrsize = sizeof(peeraddr);

    connectedfd = accept(sockfd, (struct sockaddr *)&peeraddr, &peeraddrsize);
    if (status == -1) {
        cerr << "start_listening error: accept" << endl;
        return 2;
    } else {
        cout << "Good connection!" << endl;
        comm_loop(connectedfd);
    }
    

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

    return string("NO"); 
}

int send_msg (int socketfd) {
    
    //char* msg = prompt_for_msg();

    char msg[172];
    int msglen = sizeof(msg);

    cout << "Type message: ";
    cin.getline(msg, msglen);

    int flags = 0;

    int bytes_sent = send(socketfd, msg, msglen, flags);
 
    return 0;
}

int recv_msg (int socketfd) {

    int bufferlen = 255;
    char buffer[bufferlen];
    int flags = 0;
 
    cout << "Waiting for message..." << endl;
    int bytes_received = recv(socketfd, buffer, bufferlen, flags);

    cout << "Message received: " << buffer << endl;
    cout << "Number of bytes: " << bytes_received << endl;

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

