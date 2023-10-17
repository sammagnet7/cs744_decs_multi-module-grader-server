
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
#include <vector>
#include <stdint.h>
#include<thread>

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
void removeTempFiles(vector<string> files_to_remove);




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
    int totalReceived = 0;
    int bytesLeft = 0;
    int currentLen = 0;

    uint32_t tmp,datalen;  
    ssize_t bytes_received = recv(client_socket, &tmp, sizeof tmp, 0);
    datalen = ntohl(tmp);
    
    cout<<"Thread id: "<< std::this_thread::get_id()<< ":: File size is: "<<datalen<<endl;

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
            cout <<"Thread id: "<< std::this_thread::get_id()<< ":: Partial data received of size: " << totalReceived;
            close(client_socket);
            break;
        }
        buffer[datalen] = '\0';
        string temp(buffer);
        data += temp;
        totalReceived += currentLen;
        bytesLeft -= currentLen;

        cout<<"Thread id: "<< std::this_thread::get_id()<< ":: Total data received: "<<totalReceived<<endl;
    }
    return currentLen == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, const char *buf, int datalen)
{
    int totalsent = 0;       // how many bytes we've sent
    int bytesleft = datalen; // how many we have left to send
    int currentLen;          // successfully sent data length on current pass

    uint32_t n = datalen;
    uint32_t tmp = htonl(n);
    send(socket, &tmp, sizeof(tmp), 0);

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
        cout<<"error in remove";
    }
}

// removes all the temporarily created files
void removeTempFiles(vector<string> files_to_remove)
{
    for(string file : files_to_remove){
        _remove(file);
    }
}



//<<<<<<<<<<<<<<<===================== main methods below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

string run_prog(string prog, string client_socket, vector<string>& files_to_remove)
{
    string response = "";

    // Compile the program
    string recv_filename = "received_"+client_socket+".cpp" ;
    string obj_filename = "received_"+client_socket;
    string compiler_output_filename = "compiler_output_"+client_socket+".log";
    string compile_command = "g++ -o "+obj_filename+" "+recv_filename+" > "+compiler_output_filename+" 2>&1";
    
    int compiled_status = system(compile_command.c_str());

    files_to_remove.push_back(recv_filename);
    files_to_remove.push_back(obj_filename);
    files_to_remove.push_back(compiler_output_filename);

    
    if (compiled_status != 0)
    {
        // COMPILER__ERROR
        // reads compiler error from temporary file
        string compiler_output = read_file(compiler_output_filename.c_str());
        response = "COMPILER ERROR\n" + compiler_output + "\n";

        return response;
    }

 
    // otherwise Execute program
    string prog_output_filename = "prog_output_"+client_socket+".log";
    string prog_run_command = "{ ./"+obj_filename+"; } > "+prog_output_filename+" 2>&1";
    int runtime_status = system(prog_run_command.c_str());

    // read the program's output from temporary file
    string run_output = read_file(prog_output_filename.c_str());

    files_to_remove.push_back(prog_output_filename);

    if (runtime_status != 0)
    {
        // RUNTIME__ERROR
        response = "RUNTIME ERROR\nError code=" + to_string(runtime_status) + "\n" + run_output + "\n";
        return response;
    }


    // Otherwise create temporary files to compare
    string actual_output_filename = "actual_file_"+client_socket+".log";
    string expected_output_filename = "expected_file_"+client_socket+".log";
    save_to_file(actual_output_filename, run_output);
    save_to_file(expected_output_filename, string(EXPECTED_OUTPUT));


    // Run the diff command to compare the temporary files
    string diffcheck_filename = "diffcheck_"+client_socket+".log";
    string diff_command = "diff -u "+expected_output_filename+" "+actual_output_filename+" > "+diffcheck_filename+" 2>&1";
    int ret = system(diff_command.c_str());

    files_to_remove.push_back(actual_output_filename);
    files_to_remove.push_back(expected_output_filename);
    files_to_remove.push_back(diffcheck_filename);


    if (ret == 0)
    {
        // PASS__
        response = "PASS\n";
        return response;
    }

    // OUTPUT__ERROR
    string diff_output = read_file(diffcheck_filename);
    response = "OUTPUT ERROR\n" + diff_output + "\n";
    return response;
}

void handle_client(int client_socket)
{   
    string received = "";
    // Receive the source code from the client
    if (int resp_code = receiveall(client_socket, received) != 0){
        // cleaning
        close(client_socket);
        return;
    }

    //List to add all the files being created throughout the process
    vector<string> files_to_remove;

    string recv_filename = "received_"+to_string(client_socket)+".cpp" ;
    // Save the received source code to a file
    save_to_file(recv_filename, received);

    files_to_remove.push_back(recv_filename);
    
    //evaluate
    string final_response = run_prog(received , to_string(client_socket),  files_to_remove);

    // Send response
    if (int resp_code = sendall(client_socket, final_response.c_str(), final_response.length()) != 0){
        // cleaning
        close(client_socket);
        removeTempFiles(files_to_remove);
        return;
    }

    // Closing client
    close(client_socket);
    removeTempFiles(files_to_remove);
    return;
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

    // Server listens on the socket. Here we are telling the max number of clients that can wait in the backlog queue until they are assigned with client specific socket
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

        std::thread(handle_client, client_socket).detach();

    }

    close(server_socket);

    return 0;
}