
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

const string EXPECTED_OUTPUT = "1 2 3 4 5 6 7 8 9 10 ";
const int MAX_QUEUE_SIZE = 50;


//<<<<<<<<<<<<<<<============== Utility methods declarations below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

//Calls bind at server end which returns the binded server Socket id
int bind_server_socket(int port);

// Accepts each clients Connection request at server end and gives us the connected client specific socket id
int getClientSocket(int binded_server_socket);


// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data);

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, const char *buf, int datalen);

//Reads given file into a string and returns it
string read_file(string file);

// saves the given string into the newly created file with the given filename
void save_to_file(string filename, string input);

// removes all the temporarily created files
void removeTempFiles();


//<<<<<<<<<<<<<<<===================== main methods below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

string run_prog(string prog)
{
    string response = "";

    // Compile the program
    int compile_status = system("g++ -o received received.cpp > compiler_output.txt 2>&1");

    if (compile_status != 0)
    {
        // COMPILER__ERROR
        // reads compiler error from temporary file
        string compiler_output = read_file("compiler_output.txt");
        response = "COMPILER ERROR\n" + compiler_output + "\n";

        return response;
    }

    // otherwise Execute program
    int runtime_status = system("{ ./received; } > prog_output.txt 2>&1");

    // read the program's output from temporary file
    string run_output = read_file("prog_output.txt");

    if (runtime_status != 0)
    {
        // RUNTIME__ERROR
        response = "RUNTIME ERROR\nError code=" + to_string(runtime_status) + "\n" + run_output + "\n";
        return response;
    }

    // Otherwise create temporary files to compare
    save_to_file("actual_file.txt", run_output);
    save_to_file("expected_file.txt", string(EXPECTED_OUTPUT));

    // Run the diff command to compare the temporary files
    int ret = system("diff -u expected_file.txt actual_file.txt > diffcheck.txt 2>&1");

    if (ret == 0)
    {
        // PASS__
        response = "PASS\n";
        return response;
    }

    // OUTPUT__ERROR
    string diff_output = read_file("diffcheck.txt");
    response = "OUTPUT ERROR\n" + diff_output + "\n";
    return response;
}

int handle_client(int client_socket)
{
    string received = "";
    // Receive the source code from the client
    if (int resp_code = receiveall(client_socket, received) != 0)
        return resp_code;

    // Save the received source code to a file
    save_to_file("received.cpp", received);

    //evaluate
    string final_response = run_prog(received);

    // Send response
    if (int resp_code = sendall(client_socket, final_response.c_str(), final_response.length()) != 0)
        return resp_code;

    // Closing client
    close(client_socket);
    return 0;
}

int main(int argc, char *argv[])
{
    // Describe Usage
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <port>" << endl;
        return 1;
    }
    // Extracting from cmd args
    int port = atoi(argv[1]);
    int server_socket = 0;

    // binding server socket
    if ((server_socket = bind_server_socket(port)) < 0)
        return server_socket;

    // Server listens on the socket. Here we are telling 20 clients can wait in the backlog queue until they are assigned with client specific socket
    if( listen(server_socket, MAX_QUEUE_SIZE) < 0 ){
        perror("Listen failed");
        close(server_socket);
        return -1;
    }

    cout << "Server listening on port: "<< port <<endl;

    while (true)
    {
        // gets new client socket upon 'Accept'
        int client_socket = getClientSocket(server_socket);
        if (client_socket < 0)
            continue;

        // Business logic runs here
        if (int resp_code = handle_client(client_socket) != 0)
            return resp_code;

        // cleaning memory
        close(client_socket);
        removeTempFiles();
    }
    close(server_socket);

    return 0;
}




//<<<<<<<<<<<<<<<=============== Utility methods definitions below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>
// Calls bind at server end which returns the binded server Socket id
int bind_server_socket(int port)
{

    int serverSocket = socket(PF_INET, SOCK_STREAM, 0);
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
        close(serverSocket);
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
    if (bytes_received <= 0)
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
        if(currentLen == 0){
            perror("Remote side has closed the connection");
        }
        else if (currentLen == -1)
        {
            perror("Error while receiving data at server");
            cout << "Partial data received of size: " << totalReceived;
            close(client_socket);
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
        //cout << "Total data sent from this machine: " << totalsent << endl;
    }
    return currentLen == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

//Reads given file into a string and returns it
string read_file(string file)
{
    ifstream filestream(file.c_str(), ios::binary);
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

// saves the given string into the newly created file with the given filename
void save_to_file(string filename, string input)
{
    ofstream new_file(filename);
    new_file.write(input.c_str(), input.length());
    new_file.close();
}

//Wrapper function
void _remove(string file)
{
    try
    {
        remove(file.c_str());
    }
    catch (...)
    {
    }
}

// removes all the temporarily created files
void removeTempFiles()
{
    _remove("compiler_output.txt");
    _remove("prog_output.txt");
    _remove("actual_file.txt");
    _remove("expected_file.txt");
    _remove("diffcheck.txt");
    _remove("received.cpp");
    _remove("received");
}