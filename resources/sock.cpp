#include "../headers/fileio.hpp"

using namespace std;

// Calls bind at server end which returns the binded server Socket id
int bind_server_socket(int port)
{

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        perror("Error opening socket at server");
        return -1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (::bind(serverSocket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Error binding socket at server");
        return -1;
    }
    return serverSocket;
}

// Accepts each clients Connection request at server end and gives us the connected client specific socket id
int getClientSocket(int binded_server_socket)
{
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int client_socket = accept(binded_server_socket, (struct sockaddr *)&client_address, &client_address_len);
    if (client_socket < 0)
    {
        perror("Error accepting client connection at server");
        return -1;
    }
    return client_socket;
}

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