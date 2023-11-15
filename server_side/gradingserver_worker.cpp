
    #include "gradingserver_worker.hpp"

    using namespace std;

    const string EXPECTED_OUTPUT = "1 2 3 4 5 6 7 8 9 10 ";

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

        cout << "Thread id: " << std::this_thread::get_id() << ":: File size is: " << datalen << endl;

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
            if (currentLen == 0)
            {
                perror("Remote side has closed the connection");
            }
            else if (currentLen == -1)
            {
                perror("Error while receiving data at server");
                cout << "Thread id: " << std::this_thread::get_id() << ":: Partial data received of size: " << totalReceived;
                close(client_socket);
                break;
            }
            buffer[datalen] = '\0';
            string temp(buffer);
            data += temp;
            totalReceived += currentLen;
            bytesLeft -= currentLen;

            cout << "Thread id: " << std::this_thread::get_id() << ":: Total data received: " << totalReceived << endl;
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
                cout << "Partial data sent of size: " << totalsent;
                break;
            }
            totalsent += currentLen;
            bytesleft -= currentLen;
            // cout << "Total data sent from this machine: " << totalsent << endl;
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
        string compile_command = "g++ -o " + obj_filename + " " + recv_filename + " > " + compiler_output_filename + " 2>&1";
        string timeout = "timeout --foreground 2m";
        string prog_run_command = "{ " + timeout + " ./" + obj_filename + "; } > " + prog_output_filename + " 2>&1";
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

    // this function handles the worker thread
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
        string log = "Worker started with thread Id: " + thId + " with socket Id: " + to_string(client_socket) + " and with traceId: " + traceId;
        cout << log << endl;

        string received = "";
        // Receive the source code from the client
        if (int resp_code = receiveall(client_socket, received) != 0)
        {
            // cleaning
            close(client_socket);
            return;
        }

        // Sample GradingDetails object

        details.trace_id = traceId;
        details.progress_status = "IN_QUEUE";
        details.submitted_file = received;
        details.grading_status = "PENDING";
        details.grading_output = "No output yet";

        red_util.pushBack(traceId);
        pg_util.insertGradingDetails(details);

        string response = "Your grading request ID <" + traceId + "> has been accepted and is currently being processed";

        // // Send response
        // if (int resp_code = sendall(client_socket, final_response.c_str(), final_response.length()) != 0){
        //     // cleaning
        //     close(client_socket);
        //     removeTempFiles(files_to_remove);
        //     return;
        // }

        // // Closing client
        // close(client_socket);
        // removeTempFiles(files_to_remove);
        // return;

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

    void grader_worker_handler()
    {
        Redis_util red_util;
        Postgres_util pg_util;
        GradingDetails details;

        // List to add all the files being created throughout the process
        vector<string> files_to_remove;

        // poppiing from shared queue
        std::string traceId = red_util.pullFront();

        // retrieve the entry from DB
        details = pg_util.retrieveGradingDetails(traceId);

        // modify progress status into DB
        details.progress_status = "IN_PROGRESS";
        pg_util.updateGradingDetails(details);
        
        // get current thread id
        auto threadId = this_thread::get_id();
        stringstream ss;
        ss << threadId;
        string thId = ss.str();

        string recv_filename = "received_" + thId + ".cpp";

        // evaluate
        GradingDetails grading_response = run_prog(details.submitted_file, thId, files_to_remove);

        grading_response.trace_id = traceId;
        grading_response.progress_status = "DONE";
        grading_response.submitted_file = details.submitted_file;

        pg_util.updateGradingDetails(grading_response);

        removeTempFiles(files_to_remove);
        return;
    }