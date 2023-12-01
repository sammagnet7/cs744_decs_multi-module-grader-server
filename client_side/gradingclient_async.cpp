#include "fileio.hpp"

#include <iostream>
#include <netdb.h>
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
#include <chrono>
#include <stdint.h>
#include <thread>
#include <future>
#include <pthread.h>
#include <sstream>

using namespace std;

//<<<<<<<<<<<<<<<=============== Utility methods definitions below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data_received)
{
    int totalReceived = 0;
    int bytesLeft = 0;
    int currentLen = 0;

    uint32_t tmp, datalen;
    ssize_t bytes_received = recv(client_socket, &tmp, sizeof(tmp), 0);

    if (bytes_received <= 0)
    {
        cerr << "Error while: <receiving data length>" << endl;
        close(client_socket);
        return -1;
    }

    datalen = ntohl(tmp);
    char buffer[datalen + 1];
    bytesLeft = datalen;

    while (totalReceived < datalen)
    {

        memset(buffer, 0, datalen);

        currentLen = recv(client_socket, buffer + currentLen, bytesLeft, 0);
        if (currentLen == -1)
        {
            cerr << "Error while: <receiving data>" << endl;
            break;
        }

        buffer[datalen] = '\0'; // helps while converting char array to string

        string temp(buffer);

        data_received += temp;
        totalReceived += currentLen;
        bytesLeft -= currentLen;
    }

    return currentLen == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, string buf, int datalen)
{
    int totalsent = 0;       // how many bytes we've sent
    int bytesleft = datalen; // how many we have left to send
    int currentLen;          // successfully sent data length on current pass

    uint32_t n = datalen;
    uint32_t tmp = htonl(n);

    // sends the file size first
    ssize_t sent = 0;
    if ((sent = send(socket, &tmp, sizeof(tmp), 0)) == -1)
    {
        cerr << "Error while: <sending the file size>" << endl;
        return -1;
    }

    // sends total file data
    while (totalsent < datalen)
    {
        currentLen = send(socket, buf.substr(totalsent).c_str(), bytesleft, 0);
        if (currentLen == -1)
        {
            cerr << "Error while: <sending the file>" << endl;
            break;
        }
        totalsent += currentLen;
        bytesleft -= currentLen;
    }
    return currentLen == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// sends trace_id stored in the pointed char buffer to the specified socket
int sendTraceId(int socket, string buf, int datalen)
{
    ssize_t sent = 0;

    if ((sent = send(socket, buf.c_str(), datalen, 0)) == -1)
    {
        cerr << "Error while: <sending traceId>" << endl;
        return -1;
    }

    string log = "Status check sent for TraceId: " + buf;
    cout << log << endl;
    return 0; // return -1 on failure, 0 on success
}

// Makes the CONNECT call from client end and returns the connected socket id upon successful
int connectsocketbyIpPort(string server_ip, int server_port)
{
    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Error opening socket");
        return -1;
    }

    // connect socket
    struct sockaddr_in target_server_address;
    memset(&target_server_address, 0, sizeof(target_server_address));

    target_server_address.sin_family = AF_INET;

    target_server_address.sin_port = htons(server_port);

    inet_pton(AF_INET, server_ip.c_str(), &(target_server_address.sin_addr));

    if (connect(client_socket, (struct sockaddr *)&target_server_address, sizeof(target_server_address)) < 0)
    {
        cerr << "Error while: <connecting to server>" << endl;
        return -1;
    }

    return client_socket;
}

// This function is used for Poll the service with given request id
void polling(string traceId, string server_ip, int server_port)
{
    int query_socket = 0;
    try
    {
        // Connects client to the Query server on the specified host-port
        if ((query_socket = connectsocketbyIpPort(server_ip, server_port)) < 0)
            throw query_socket;

        // Send the file iteratively
        if (int resp_code = sendTraceId(query_socket, traceId.c_str(), traceId.length()) != 0)
            throw resp_code;

        // Receive the grading_response
        string grading_response = "";
        if (int resp_code = receiveall(query_socket, grading_response) != 0)
            throw resp_code;

        cout << grading_response << endl;
    }
    catch (...)
    {
        perror("EXCEPTION occured while polling: ");
    }

    close(query_socket);
}

void submitting(string file_name, string server_ip, int server_port)
{
    int submission_client_socket = 0;
    try
    {
        // Connects client to the server on the specified host-port
        if ((submission_client_socket = connectsocketbyIpPort(server_ip, server_port)) < 0)
            throw submission_client_socket;

        // Reads the file into a String
        string source_code = read_file(file_name);

        // Send the file data to the client iteratively
        if (int resp_code = sendall(submission_client_socket, source_code.c_str(), source_code.length()) != 0)
            throw resp_code;

        // Receive the source code from the client
        string submition_response = "";
        if (int resp_code = receiveall(submission_client_socket, submition_response) != 0)
            throw resp_code;

        cout << submition_response << endl;
    }
    catch (...)
    {
        perror("EXCEPTION occured while submitting: ");
    }
    close(submission_client_socket);
}

//<<<<<<<<<<<<<<<===================== main method below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

int main(int argc, char *argv[])
{

    // Describe usage
    if (argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <new|status> <submission_serverIP:port> <sourceCodeFileTobeGraded|requestID>" << endl;
        return 1;
    }

    // Extract all the command line args
    string trigger = argv[1];
    
    hostent *record = gethostbyname(strtok(argv[2], ":"));
    if (record == NULL)
    { // hostname to ip resolution
        printf("%s is unavailable\n", strtok(argv[2], ":"));
        exit(1);
    }
    in_addr *address = (in_addr *)record->h_addr;
    string server_ip = inet_ntoa(*address);

    int server_port = stoi(strtok(NULL, ":"));

    /** This block is to handle new grading request submission logic */
    if (trigger == "new")
    {
        string file_name = argv[3];

        submitting(file_name, server_ip, server_port);
    }
    /** This block is to handle polling request for given request id */
    else if (trigger == "status")
    {
        string requestID = argv[3];

        polling(requestID, server_ip, server_port);
    }
    else
    {
        cerr << "Wrong trigger given: " << trigger;
    }

    return 0;
}
