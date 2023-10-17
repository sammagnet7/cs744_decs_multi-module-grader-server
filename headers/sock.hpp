// #pragma once
#ifndef SOCK_H
#define SOCK_H

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>

using namespace std;

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, const char *buf, int datalen);

// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data);

//Calls bind at server end which returns the binded server Socket id
int bind_server_socket(int port);

// Accepts each clients Connection request at server end and gives us the connected client specific socket id
int getClientSocket(int binded_server_socket);

// makes the CONNECT call from client end and returns the connected socket id upon successful
int connectsocketbyIpPort(char *server_ip, int server_port);

#endif