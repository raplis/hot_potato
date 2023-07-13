#include <stdio.h>
#include <cstdlib>
#include <netdb.h>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#define ADDR_LEN 30

using namespace std;

// This function creates a server socket and binds it to a given port
// It returns the socket file descriptor or exits with an error message
int s_before_acc(const char *port) {
    // Create a socket using IPv4 and TCP protocol
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    // Check if the socket creation was successful
    if(sock_fd == -1) {
        fprintf(stderr, "failure on socket establish.\n");
        exit(EXIT_FAILURE);
    }
    // Initialize a sockaddr_in structure to store the server address
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    const char *temp = "-1";
    // If the port is -1, use any available port
    if(strcmp(temp, port) == 0) {
        address.sin_port = htons(0);
    }
    else {  // Otherwise, use the given port number
        address.sin_port = htons(atoi(port));
    }
    // Bind the socket to the server address
    int num = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &num, sizeof(int));
    int ret = bind(sock_fd, (struct sockaddr*) &address, sizeof(address));
    if(ret == -1) { // Check if the binding was successful
        fprintf(stderr, "failure on bind establish.\n");
        exit(EXIT_FAILURE);
    }
    // Listen for incoming connections with a backlog of 100
    ret = listen(sock_fd, 100);
    if(ret == -1) {     // Check if the listening was successful
        fprintf(stderr, "failure on listen establish.\n");
        exit(EXIT_FAILURE);
    }
    return sock_fd;
}
// This function accepts a connection from a client and IP address as a string reference 
//and returns its socket file descriptor 
int s_acc(int sock_fd, string& ip) {
    // Initialize a sockaddr_in structure to store the client address 
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    // Accept a connection from the client and get its socket file descriptor 
    int client_sock = accept(sock_fd, (struct sockaddr*)&client, &client_len);
    if(client_sock == -1) { // Check if the acceptance was successful 
        fprintf(stderr, "accept failure.\n");
        exit(EXIT_FAILURE);
    }
    // Convert the client IP address from binary to text format and store it in ip 
    ip = string(inet_ntoa(client.sin_addr));
    return client_sock;
}

// This function creates a client socket and connects it to a given host name and port number 
// It returns the socket file descriptor or exits with an error message
int c_before_con(const char* port_num, const char* hostname) {
    // Initialize an addrinfo structure to store hints for getting host information 
    struct addrinfo host;
    struct addrinfo *host_list;
    memset(&host, 0, sizeof(host));
    host.ai_family = AF_INET;
    host.ai_socktype = SOCK_STREAM;
    // Get host information using getaddrinfo function 
    int ret = getaddrinfo(hostname, port_num, &host, &host_list);
    // cout << hostname << "  " << port_num << endl;
    if(ret != 0) {
        fprintf(stderr, "failure on client getaddrinfo.\n");
        exit(EXIT_FAILURE);
    }
    // Create a socket using IPv4 and TCP protocol
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1) {
        fprintf(stderr, "failure on client socket establish.\n");
        exit(EXIT_FAILURE);
    }
    // Connect to the host using its address information 
    ret = connect(sock_fd, host_list->ai_addr , host_list->ai_addrlen);
    if(ret == -1) {
        fprintf(stderr, "failure on connect.\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(host_list);
    return sock_fd;
}
