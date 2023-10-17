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

using namespace std;

//<<<<<<<<<<<<<<<============== Utility methods declarations below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data);

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, string buf, int datalen);

// makes the CONNECT call from client end and returns the connected socket id upon successful
int connectsocketbyIpPort(string server_ip, int server_port);

// Reads given file into a string and returns it
string read_file(string ile);

//<<<<<<<<<<<<<<<===================== main method below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

int main(int argc, char *argv[])
{

    // Describe usage
    if (argc != 5)
    {
        cerr << "Usage: " << argv[0] << " <serverIP:port> <sourceCodeFileTobeGraded> <loopNum> <sleepTimeSeconds>" << endl;
        return 1;
    }

    // Extract all the command line args
    string server_ip = strtok(argv[1], ":");
    int server_port = stoi(strtok(NULL, ":"));
    string file_name = argv[2];
    int loopNum = stoi(argv[3]);
    int sleepTime_sec = stoi(argv[4]);

    int client_socket = 0;
    double accumulated_time = 0;
    int successCount = 0;

    // Note: LOOP START time
    auto LStart = chrono::high_resolution_clock::now();
    while (loopNum--)
    {
        try
        {
            // Connects client to the server on the specified host-port
            if ((client_socket = connectsocketbyIpPort(server_ip, server_port)) < 0)
                throw client_socket;

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
            double time_taken = chrono::duration_cast<chrono::milliseconds>(Trecv - Tsend).count();

            cout << server_response << endl;

            accumulated_time += time_taken;
            successCount++;
        }
        catch (...)
        {
            // catches any exceptions
            cout << "EXCEPTION occured inside loop" << endl;
        }
        close(client_socket);
        sleep(sleepTime_sec);
    }
    // Note: LOOP END time
    auto LEnd = chrono::high_resolution_clock::now();

    // Calculating total time taken by the loop.
    double loop_time = chrono::duration_cast<chrono::milliseconds>(LEnd - LStart).count();

    double avgRespTime = successCount > 0 ? (accumulated_time / successCount) : 0;
    double throughput_p_sec = successCount > 0 ? ( (successCount/loop_time)*1000 ) : 0;

    cout << "-------------------per client basis-------------------------------" << endl;
    cout << "Accumulated response time (in ms) :" << accumulated_time << endl;
    cout << "Average response time (in ms) :" << avgRespTime << endl;
    cout << "Number of successful responses :" << successCount << endl;
    cout << "Time taken for completing client loop (in ms) :" << loop_time << endl;
    cout << "Individual client throughput per seconds :" << throughput_p_sec << endl << endl;
    return 0;
}

//<<<<<<<<<<<<<<<=============== Utility methods definitions below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data_received)
{
    int datalen = 0;
    int totalReceived = 0;
    int bytesLeft = 0;
    int currentLen = 0;

    ssize_t bytes_received = recv(client_socket, &datalen, sizeof(int), 0);
    if (bytes_received <= 0)
    {
        perror("Error while receiving data length");
        close(client_socket);
        return -1;
    }

    char buffer[datalen];
    bytesLeft = datalen;
    while (totalReceived < datalen)
    {

        memset(buffer, 0, datalen);

        currentLen = recv(client_socket, buffer + currentLen, bytesLeft, 0);
        if(currentLen == 0){
            perror("Remote side has closed the connection");
        }
        else if (currentLen == -1)
        {
            perror("Error while receiving data");
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

    // sends the file size to be sent
    if (send(socket, &datalen, sizeof(datalen), 0) == -1)
    {
        perror("Error while sending file size");
        return -1;
    }

    // sends total file data
    while (totalsent < datalen)
    {
        currentLen = send(socket, buf.substr(totalsent).c_str(), bytesleft, 0);
        if (currentLen == -1)
        {
            perror("Error while sending the data");
            break;
        }
        totalsent += currentLen;
        bytesleft -= currentLen;
        // std::cout << "Total data sent from this machine: " << totalsent << endl;
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

    struct sockaddr_in target_server_address;
    memset(&target_server_address, 0, sizeof(target_server_address));

    target_server_address.sin_family = AF_INET;

    target_server_address.sin_port = htons(server_port);

    inet_pton(AF_INET, server_ip.c_str(), &(target_server_address.sin_addr));

    if (connect(client_socket, (struct sockaddr *)&target_server_address, sizeof(target_server_address)) < 0)
    {
        perror("Error connecting to server");
        return -1;
    }
    return client_socket;
}

// Reads given file into a string and returns it
string read_file(string file)
{
    ifstream filestream(file, ios::binary);
    string filedata((istreambuf_iterator<char>(filestream)),
                    istreambuf_iterator<char>());
    /*
     string txt, txt_accumulator;
     while (getline(filestream, txt))
     {
         txt_accumulator += (txt + "\n");
     }
     */
    filestream.close();
    return filedata;
}
