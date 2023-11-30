#include "fileio.hpp"

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
#include <chrono>
#include <stdint.h>
#include <thread>
#include <future>
#include <pthread.h>

using namespace std;

string server_ip;
int server_port;
string file_name;
float timeout_seconds;


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

    char buffer[datalen];
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

// Makes the CONNECT call from client end and returns the connected socket id upon successful
int connectsocketbyIpPort(string server_ip, int server_port)
{
    int client_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
    {
        perror("Error opening socket");
        return -1;
    }

    //set socket timeout
     struct timeval timeout;
     timeout.tv_sec = (int)timeout_seconds;
     timeout.tv_usec = (timeout_seconds - timeout.tv_sec) * 1000000;

    if (setsockopt (client_socket, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                sizeof timeout) < 0)
        perror("setsockopt for SENDTIMO failed\n");

    if (setsockopt (client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                sizeof timeout) < 0)
        perror("setsockopt RCVTIMO failed\n");

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

void worker(int *sockfd, bool *_isSuccess, bool *_isTimedOut, double *_tx_Rx_time)
{
    int client_socket = 0;
    try
    {
        // Connects client to the server on the specified host-port
        if ((client_socket = connectsocketbyIpPort(server_ip, server_port)) < 0)
            throw client_socket;

        // Puttng value into pointer
        *sockfd = client_socket;
        // Reads the file into a String
        string source_code = read_file(file_name);

        // Note: data TRANSMISSION START time
        auto Tsend = chrono::high_resolution_clock::now();

        // Send the file data to the client iteratively
        if (int resp_code = sendall(client_socket, source_code.c_str(), source_code.length()) != 0)
            throw resp_code;

        // Receive the source code from the client
        string server_response = "";
        if (int resp_code = receiveall(client_socket, server_response) != 0)
            throw resp_code;

        // Note: data RECEIVING END time
        auto Trecv = chrono::high_resolution_clock::now();

        // Calculating total time taken for transmit and receive.
        *_tx_Rx_time = chrono::duration_cast<chrono::milliseconds>(Trecv - Tsend).count();

        cout << server_response << endl;

        *_isSuccess = true;
    }
    catch (...)
    {   
        *_isSuccess = false;
        // if this worker thread is already timed out by the main thread and detached, don't print anything
        if (!(*_isTimedOut))
        {
            // catches any exceptions
            cout << "EXCEPTION occured inside loop" << endl;
            perror("Exception is: ");
        }
    }
}

//<<<<<<<<<<<<<<<===================== main method below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

int main(int argc, char *argv[])
{

    // Describe usage
    if (argc != 6)
    {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds> <timeout-seconds>" << endl;
        return 1;
    }

    // Extract all the command line args
    server_ip = strtok(argv[1], ":");
    server_port = stoi(strtok(NULL, ":"));
    file_name = argv[2];
    timeout_seconds = stof(argv[5]);
    
    int loopNum = stoi(argv[3]);
    float sleepTime_sec = stof(argv[4]);

    double totalCount = loopNum;
    double successCount = 0;
    double timeout_count = 0;
    double other_err_count = 0;
    double accumulated_time = 0;

    // Note: LOOP START time
    auto LStart = chrono::high_resolution_clock::now();
    while (loopNum--)
    {
        cout << "------------ New Submission ------------------------" << endl;
        bool *_isSuccess = new bool(false);
        bool *_isTimedOut = new bool(false);

        double *_tx_Rx_time = new double(0);
        int *client_socket = new int();

        /* Creating threads */
        std::thread th(worker, client_socket, _isSuccess, _isTimedOut, _tx_Rx_time);

        auto future = std::async(std::launch::async, &std::thread::join, &th);
        if (future.wait_for(std::chrono::milliseconds((int) (timeout_seconds * 1000) )) == std::future_status::timeout)
        {
            /* do things, if thread has not terminated within time */
            *_isTimedOut = true;
            close(*client_socket);

            timeout_count++;
        }
        else
        {
            /* No timeout occured */
            if (*_isSuccess)
            {
                accumulated_time += (*_tx_Rx_time);
                successCount++;
            }
            else
                other_err_count++;
        }
        client_socket = NULL;
        _isSuccess = NULL;
        _isTimedOut = NULL;
        _tx_Rx_time = NULL;

        sleep(sleepTime_sec);
    }

    // Note: LOOP END time
    auto LEnd = chrono::high_resolution_clock::now();

    // Calculating total time taken by the loop.
    double turnAroundTime = chrono::duration_cast<chrono::milliseconds>(LEnd - LStart).count();

    double avgRespTime = successCount > 0 ? (accumulated_time / successCount) : 0;

    double totalCount_p_sec = totalCount > 0 ? ((totalCount / turnAroundTime) * 1000) : 0;
    double throughput_p_sec = successCount > 0 ? ((successCount / turnAroundTime) * 1000) : 0;
    double timeout_count_p_sec = timeout_count > 0 ? ((timeout_count / turnAroundTime) * 1000) : 0;
    double other_err_count_p_sec = other_err_count > 0 ? ((other_err_count / turnAroundTime) * 1000) : 0;

    cout << "============================== per client basis stats ======================================" << endl;
    cout << "Accumulated response time (in sec) :" << (accumulated_time / 1000) << endl;
    cout << "Average response time (in sec) :" << (avgRespTime / 1000) << endl;

    // request sent should be == throughput + timeout + error rate
    cout << "Number of total requests sent :" << totalCount << endl;           // total requests
    cout << "Number of successful responses :" << successCount << endl;        // throughput
    cout << "Number of timeout requests :" << timeout_count << endl;           // timeout
    cout << "Number of all other error requests :" << other_err_count << endl; // error

    cout << "Turn Around Time or loop time (in sec) :" << (turnAroundTime / 1000) << endl;
    cout << "Individual client total requests per seconds :" << totalCount_p_sec << endl;
    cout << "Individual client throughput per seconds :" << throughput_p_sec << endl;
    cout << "Individual client timeout requests per seconds :" << timeout_count_p_sec << endl;
    cout << "Individual client other error requests per seconds :" << other_err_count_p_sec << endl
         << endl;

    return 0;
}
