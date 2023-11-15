
#include "fileio.hpp"

#include <cstdlib>
#include <cstring>
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

#include "redis_util.hpp"
#include "postgres_util.hpp"
using namespace std;


// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data);

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, string buf, int datalen);


//the submitted file is run using g++ compiler
GradingDetails run_prog(string recvd_string, string thread_id, vector<string>& files_to_remove);

//this function handles the worker thread
void submission_worker_handler(int client_socket, string traceId);

void grader_worker_handler();