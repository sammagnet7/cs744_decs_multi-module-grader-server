#include "headers/sock.hpp"
#include "headers/fileio.hpp"

const int MAX_BUFFER_SIZE = 1024;

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
