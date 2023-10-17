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

//<<<<<<<<<<<<<<<============== Utility methods declarations below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>


// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data);

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, const char *buf, int datalen);

// makes the CONNECT call from client end and returns the connected socket id upon successful
int connectsocketbyIpPort(char *server_ip, int server_port);

//Reads given file into a string and returns it
string read_file(char *file);


//<<<<<<<<<<<<<<<===================== main method below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

int main(int argc, char *argv[])
{
    // Describe usage
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded>" << endl;
        return 1;
    }

    // Extract IP and Port from args
    char *server_ip = strtok(argv[1], ":");
    int server_port = atoi(strtok(NULL, ":"));
    int client_socket = 0;

    // Connects client to the server on the specified host-port
    if ((client_socket = connectsocketbyIpPort(server_ip, server_port)) < 0)
        return client_socket;

    // Reads the file into a String
    string source_code = read_file(argv[2]);

    // Send the file data to the client iteratively
    if (int resp_code = sendall(client_socket, source_code.c_str(), source_code.length()) != 0)
        return resp_code;

    string server_response = "";
    // Receive the source code from the client
    if (int resp_code = receiveall(client_socket, server_response) != 0)
        return resp_code;

    cout << server_response<<endl;

    close(client_socket);
    return 0;
}


//<<<<<<<<<<<<<<<=============== Utility methods definitions below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data)
{
    int datalen = 0;
    int totalReceived = 0;
    int bytesLeft = 0;
    int currentLen = 0;

    ssize_t bytes_received = recv(client_socket, &datalen, sizeof(int), 0);
    if (bytes_received < 0)
    {
        perror("Error receiving source code");
        close(client_socket);
        return -1;
    }
    char buffer[datalen];
    bytesLeft = datalen;

    while (totalReceived < datalen)
    {

        memset(buffer, 0, datalen);

        currentLen = recv(client_socket, buffer, bytesLeft, 0);
        if (currentLen == -1)
        {
            perror("Error while receiving data at server");
            cout << "Partial data received of size: " << totalReceived;
            break;
        }
        buffer[datalen] = '\0';
        string temp(buffer);
        data += temp;
        totalReceived += currentLen;
        bytesLeft -= currentLen;
    }
    return currentLen == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, const char *buf, int datalen)
{
    int totalsent = 0;       // how many bytes we've sent
    int bytesleft = datalen; // how many we have left to send
    int currentLen;          // successfully sent data length on current pass

    // uint32_t len = htonl(datalen);
    send(socket, &datalen, sizeof(datalen), 0);

    while (totalsent < datalen)
    {
        currentLen = send(socket, buf + totalsent, bytesleft, 0);
        if (currentLen == -1)
        {
            perror("Error while sending data to the server");
            cout << "Partial data sent of size: " << totalsent;
            break;
        }
        totalsent += currentLen;
        bytesleft -= currentLen;
        cout << "Total data sent from this machine: " << totalsent << endl;
    }
    return currentLen == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// Makes the CONNECT call from client end and returns the connected socket id upon successful
int connectsocketbyIpPort(char *server_ip, int server_port)
{
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Error opening socket");
        return -1;
    }

    struct sockaddr_in target_server_address;
    memset(&target_server_address, 0, sizeof(target_server_address));

    target_server_address.sin_family = AF_INET;

    target_server_address.sin_port = htons(server_port);

    inet_pton(AF_INET, server_ip, &(target_server_address.sin_addr));

    if (connect(client_socket, (struct sockaddr *)&target_server_address, sizeof(target_server_address)) < 0)
    {
        perror("Error connecting to server");
        return -1;
    }
    return client_socket;
}

//Reads given file into a string and returns it
string read_file(char *file)
{
    ifstream filestream(file, ios::binary);
    string filedata((istreambuf_iterator<char>(filestream)),
                       istreambuf_iterator<char>());
    string txt, txt_accumulator;
    // while (getline(filestream, txt))
    // {
    //     txt_accumulator += (txt + "\n");
    // }
    filestream.close();
    return filedata;
}

