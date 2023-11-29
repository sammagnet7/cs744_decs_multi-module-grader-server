
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

using namespace std;


// receives all incoming data associated with given socket into the pointed string
int receiveall(int client_socket, string &data);

// sends all the data stored in the pointed char buffer to the specified socket
int sendall(int socket, string buf, int datalen);


//the submitted file is run using g++ compiler
string run_prog(string prog, string client_socket, vector<string>& files_to_remove);

//this function handles the worker thread
void worker_handler(int client_socket);

//logs service time taken, into log file
void logServiceTime( long serviceTime );