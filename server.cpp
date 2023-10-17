#include "headers/sock.hpp"
#include "headers/fileio.hpp"

using namespace std;

const char *EXPECTED_OUTPUT = "1 2 3 4 5 6 7 8 9 10 ";

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
        response = "COMPILER ERROR\n\n" + compiler_output + "\n";

        return response;
    }

    // otherwise Execute program
    int runtime_status = system("{ ./received; } > prog_output.txt 2>&1");

    // read the program's output from temporary file
    string run_output = read_file("prog_output.txt");

    if (runtime_status != 0)
    {
        // RUNTIME__ERROR
        response = "RUNTIME ERROR\nError code=" + to_string(runtime_status) + "\n\n" + run_output + "\n";
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
        response = "PASS\n\nOutput:\n" + run_output + "\n";
        return response;
    }

    // OUTPUT__ERROR
    string diff_output = read_file("diffcheck.txt");
    response = "OUTPUT ERROR\n\n" + diff_output + "\n";
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

    // Server listens on the socket. Here we are telling 5 clients can wait in the backlog queue until they are assigned with client specific socket
    listen(server_socket, 5);

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
