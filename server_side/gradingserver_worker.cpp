
#include "gradingserver_worker.hpp"
#include "thread_pool.hpp"

using namespace std;

const string EXPECTED_OUTPUT = "1 2 3 4 5 6 7 8 9 10";

//<<<<<<<<<<<<<<<=============== Utility methods definitions below =============>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data)
{
    int totalReceived = 0;
    int bytesLeft = 0;
    int currentLen = 0;

    uint32_t tmp, datalen;
    ssize_t bytes_received = recv(client_socket, &tmp, sizeof tmp, 0);
    datalen = ntohl(tmp);

    if (bytes_received <= 0)
    {
        perror("Error receiving source code");
        close(client_socket);
        return -1;
    }
    char buffer[datalen+1];
    bytesLeft = datalen;
    while (totalReceived < datalen)
    {
        memset(buffer, 0, datalen);

        currentLen = recv(client_socket, buffer, bytesLeft, 0);
        if (currentLen == 0)
        {
            perror("Remote side has closed the connection");
        }
        else if (currentLen == -1)
        {
            perror("Error while receiving data at server");
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
int sendall(int socket, string buf, int datalen)
{
    int totalsent = 0;       // how many bytes we've sent
    int bytesleft = datalen; // how many we have left to send
    int currentLen;          // successfully sent data length on current pass

    uint32_t n = datalen;
    uint32_t tmp = htonl(n);
    send(socket, &tmp, sizeof(tmp), 0);

    while (totalsent < datalen)
    {
        currentLen = send(socket, buf.substr(totalsent).c_str(), bytesleft, 0);
        if (currentLen == -1)
        {
            perror("Error while sending data to the server");
            break;
        }
        totalsent += currentLen;
        bytesleft -= currentLen;
    }
    return currentLen == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

// the submitted file is run using g++ compiler
string run_prog(string prog, string client_socket, vector<string> &files_to_remove)
{
    string response = "";

    // Compile the program
    string recv_filename = "received_" + client_socket + ".cpp";
    string obj_filename = "received_" + client_socket;
    string compiler_output_filename = "compiler_output_" + client_socket + ".log";
    string compile_command = "g++ -o " + obj_filename + " " + recv_filename + " > " + compiler_output_filename + " 2>&1";

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
    string timeout = "timeout --foreground 1m";
    string prog_output_filename = "prog_output_" + client_socket + ".log";
    string prog_run_command = "{ " + timeout + " ./" + obj_filename + "; } > " + prog_output_filename + " 2>&1";
    int runtime_status = system(prog_run_command.c_str());

    // read the program's output from temporary file
    string run_output = read_file(prog_output_filename.c_str());

    files_to_remove.push_back(prog_output_filename);

    if (runtime_status != 0)
    {
        // RUNTIME__ERROR
        if (runtime_status == 31744)
            run_output = "Timeout happened while executing the submitted code";
        response = "RUNTIME ERROR\nError code=" + to_string(runtime_status) + "\n" + run_output + "\n";
        return response;
    }

    // Otherwise create temporary files to compare
    string actual_output_filename = "actual_file_" + client_socket + ".log";
    string expected_output_filename = "expected_file_" + client_socket + ".log";
    save_to_file(actual_output_filename, run_output);
    save_to_file(expected_output_filename, string(EXPECTED_OUTPUT));

    // Run the diff command to compare the temporary files
    string diffcheck_filename = "diffcheck_" + client_socket + ".log";
    string diff_command = "diff -u " + expected_output_filename + " " + actual_output_filename + " > " + diffcheck_filename + " 2>&1";
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

// this function handles the worker thread
void worker_handler(int client_socket)
{
    string received = "";
    // Receive the source code from the client
    if (int resp_code = receiveall(client_socket, received) != 0)
    {
        // cleaning
        close(client_socket);
        return;
    }

    // List to add all the files being created throughout the process
    vector<string> files_to_remove;

    string recv_filename = "received_" + to_string(client_socket) + ".cpp";
    // Save the received source code to a file
    save_to_file(recv_filename, received);

    files_to_remove.push_back(recv_filename);

    auto serviceStart = chrono::high_resolution_clock::now();

    // evaluate
    string final_response = run_prog(received, to_string(client_socket), files_to_remove);

    auto serviceEnd = chrono::high_resolution_clock::now();

    // Calculating total time taken for servicing one request
    double serviceTime = chrono::duration_cast<chrono::milliseconds>(serviceEnd - serviceStart).count();

    //Calling function from Thread pool for logging the service time
    Thread_pool th_pool;
    th_pool.logServiceTime((long)serviceTime);

    // Send response
    if (int resp_code = sendall(client_socket, final_response.c_str(), final_response.length()) != 0)
    {
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
