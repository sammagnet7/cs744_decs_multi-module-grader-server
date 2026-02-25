
#include "gradingserver_worker.hpp"

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

// receives trace_id associated with given socket into the pointed string
int receiveId(int client_socket, string &trace_id)
{
    uint32_t datalen = 4*sizeof(long);
    char buffer[datalen+1];


    ssize_t bytes_received = recv(client_socket, buffer, datalen, 0);


    if (bytes_received <= 0)
    {
        perror("Error receiving Statuscheck trace_id");
        close(client_socket);
        return -1;
    }
    buffer[bytes_received] = '\0';
    string temp(buffer);
    trace_id = temp;

    /*LOGGING*/
    auto threadId = this_thread::get_id();
    stringstream ss;
    ss << threadId;
    string thId = ss.str();
    string log = "Thread Id: " + thId + " :: StatusCheck request received for trace_id: " + trace_id;
    logMessageToFile(log);

    return 0; // return -1 on failure, 0 on success
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
GradingDetails run_prog(string recvd_string, string thread_id, vector<string> &files_to_remove)
{
    GradingDetails details;

    // making all the temp file names
    string recv_filename = "received_" + thread_id + ".cpp";
    string obj_filename = "received_" + thread_id;
    string compiler_output_filename = "compiler_output_" + thread_id + ".log";
    string prog_output_filename = "prog_output_" + thread_id + ".log";
    string actual_output_filename = "actual_file_" + thread_id + ".log";
    string expected_output_filename = "expected_file_" + thread_id + ".log";
    string diffcheck_filename = "diffcheck_" + thread_id + ".log";

    // making all the commands
    string RUN_TIMEOUT = "timeout --foreground 1m";
    string compile_command = "g++ -o " + obj_filename + " " + recv_filename + " > " + compiler_output_filename + " 2>&1";
    string prog_run_command = "{ " + RUN_TIMEOUT + " ./" + obj_filename + "; } > " + prog_output_filename + " 2>&1";
    string diff_command = "diff -u " + expected_output_filename + " " + actual_output_filename + " > " + diffcheck_filename + " 2>&1";

    // adds the temp files to the removing list
    files_to_remove.push_back(recv_filename);
    files_to_remove.push_back(obj_filename);
    files_to_remove.push_back(compiler_output_filename);
    files_to_remove.push_back(prog_output_filename);
    files_to_remove.push_back(actual_output_filename);
    files_to_remove.push_back(expected_output_filename);
    files_to_remove.push_back(diffcheck_filename);

    // Save the received source code to a file
    save_to_file(recv_filename, recvd_string);

    // Compile the program
    int compiled_status = system(compile_command.c_str());

    if (compiled_status != 0)
    {
        /*COMPILER__ERROR*/
        // reads compiler error from temporary file
        string compiler_output = read_file(compiler_output_filename.c_str());

        details.grading_status = "COMPILER ERROR";
        details.grading_output = compiler_output;

        return details;
    }

    // otherwise Execute program
    int runtime_status = system(prog_run_command.c_str());

    // read the program's output from temporary file
    string run_output = read_file(prog_output_filename.c_str());

    if (runtime_status != 0)
    {
        /*RUNTIME__ERROR*/
        if (runtime_status == 31744)
            run_output = "Timeout happened while executing the submitted code";

        details.grading_status = "RUNTIME ERROR";
        details.grading_output = "Error code=" + to_string(runtime_status) + "\n" + run_output + "\n";
        return details;
    }

    // Otherwise create temporary files to compare
    save_to_file(actual_output_filename, run_output);
    save_to_file(expected_output_filename, string(EXPECTED_OUTPUT));

    // Run the diff command to compare the temporary files
    int ret = system(diff_command.c_str());

    if (ret == 0)
    {
        /*PASS__*/
        details.grading_status = "PASS";
        return details;
    }

    /*OUTPUT__ERROR*/
    string diff_output = read_file(diffcheck_filename);
    details.grading_status = "OUTPUT ERROR";
    details.grading_output = diff_output;
    return details;
}

// this function handles the worker thread for Submission server
void submission_worker_handler(int client_socket, string traceId)
{
    Redis_util red_util;
    Postgres_util pg_util;
    GradingDetails details;

    /*LOGGING*/
    auto threadId = this_thread::get_id();
    stringstream ss;
    ss << threadId;
    string thId = ss.str();
    string log = "Submission Worker started with thread Id: " + thId + " with socket Id: " + to_string(client_socket) + " and with traceId: " + traceId;
    logMessageToFile(log);

    string received = "";
    // Receive the source code from the client
    if (int resp_code = receiveall(client_socket, received) != 0)
    {
        // cleaning
        close(client_socket);
        return;
    }

    // Received request is being queued for grading in future
    details.trace_id = traceId;
    details.progress_status = "IN_QUEUE";
    details.submitted_file = received;
    details.grading_status = "NIL";
    details.grading_output = "NIL";

    //Insert into DB and push to queue for future evaluation
    pg_util.insertGradingDetails(details);
    red_util.pushBack(traceId);

    string response = "Your grading request ID <" + traceId + "> has been accepted and is currently being processed";

    // Send response
    if (int resp_code = sendall(client_socket, response.c_str(), response.length()) != 0)
    {
        // cleaning
        close(client_socket);
        return;
    }
    // Closing client
    close(client_socket);
    return;
}

// this function handles the worker thread for Grader server
void grader_worker_handler()
{
    Redis_util red_util;
    Postgres_util pg_util;
    GradingDetails details;

    /*LOGGING*/
    auto threadId = this_thread::get_id();
    stringstream ss;
    ss << threadId;
    string thId = ss.str();
    string log = "Grader Worker started with thread Id: " + thId;
    logMessageToFile(log);

    // List to add all the files being created throughout the process
    vector<string> files_to_remove;

    // Pop from shared queue
    std::string traceId = red_util.pullFront();

    // retrieve the entry from DB
    details = pg_util.retrieveGradingDetails(traceId);

    // modify progress status into DB
    details.progress_status = "IN_GCC";
    pg_util.updateGradingDetails(details);

    // Actual GCC evaluation happen here
    GradingDetails grading_response = run_prog(details.submitted_file, thId, files_to_remove);

    //Updating status
    grading_response.trace_id = traceId;
    grading_response.progress_status = "DONE";
    grading_response.submitted_file = details.submitted_file;

    //Update to DB
    pg_util.updateGradingDetails(grading_response);

    //Free server memory
    removeTempFiles(files_to_remove);
    return;
}

// this function handles the worker thread for statusCheck server
void statusCheck_worker_handler(int client_socket)
{
    Redis_util red_util;
    Postgres_util pg_util;
    GradingDetails details;
    string response;

    /*LOGGING*/
    auto threadId = this_thread::get_id();
    stringstream ss;
    ss << threadId;
    string thId = ss.str();
    string log = "StatusCheck Worker started with thread Id: " + thId + " with socket Id: " + to_string(client_socket);
    logMessageToFile(log);

    string received_trace_id = "";

    // Receive the source code from the client
    if (int resp_code = receiveId(client_socket, received_trace_id) != 0)
    {
        // cleaning
        close(client_socket);
        return;
    }

    //Get the details from DB w.r.t trace_id
    details = pg_util.retrieveGradingDetails(received_trace_id);


    /*Step by step categorize Grading details*/
    if (details.trace_id == "0")
    {
        response = "Grading request <" + received_trace_id + "> not found. Please check and resend your request ID or re-send your original grading request.\n";
    }
    else if (details.progress_status == "IN_QUEUE")
    {
        int pos = red_util.getPos(details.trace_id);

        /*LOGGING*/
        auto threadId = this_thread::get_id();
        stringstream ss;
        ss << threadId;
        string thId = ss.str();
        string log = "Thread Id: " + thId + " with socket Id: " + to_string(client_socket) + " :: TraceId: " +details.trace_id + " Position found in shared queue: " + to_string(pos);
        logMessageToFile(log);

        if(pos == -1){
            response = "Your grading request ID <" + details.trace_id + "> has been accepted. It is currently in evaluation process.\n";
        }
        else{
            response = "Your grading request ID <" + details.trace_id + "> has been accepted. It is currently at position <" +to_string(pos) + "> in the queue.\n";
        }
    }
    else if (details.progress_status == "IN_GCC")
    {
        response = "Your grading request ID <" + details.trace_id + "> has been accepted. It is currently in evaluation process.\n";
    }
    else if (details.progress_status == "DONE")
    {
        response = "Your grading request ID <" + details.trace_id + "> processing is done, here are the results: \n" +details.grading_status + "\n" + details.grading_output + "\n";
    }
    else{
        response ="Error while retrieving data from DB\n";

    }

    logMessageToFile(response);
    
    // Send response
    if (int resp_code = sendall(client_socket, response.c_str(), response.length()) != 0)
    {
        // cleaning
        close(client_socket);
        return;
    }
    // Closing client
    close(client_socket);
    return;
}

//This returns the shared queue length
long long getSharedQueueLength()
{
    Redis_util red_util;
    return red_util.getLength();
}