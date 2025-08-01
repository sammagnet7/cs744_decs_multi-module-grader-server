
//#include "gradingserver_worker.hpp"
#include "thread_pool.hpp"

#include <iostream>
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


const int MAX_BACKLOG_QUEUE_SIZE = 1;


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


//<<<<<<<<<<<<<<<===================== main method below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

int main(int argc, char *argv[])
{
    // Describe Usage
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " <port> <thread_pool_size>" << endl;
        return 1;
    }
    // Extracting from cmd args
    int port = atoi(argv[1]);
    int server_socket = 0;
    int thread_pool_size = atoi(argv[2]);

    // binding server socket
    if ((server_socket = bind_server_socket(port)) < 0)
        return server_socket;

    // Server listens on the socket. Here we are telling the max number of clients that can wait in the backlog queue until they are assigned with client specific socket
    if( listen(server_socket, MAX_BACKLOG_QUEUE_SIZE) < 0 ){
        perror("Listen failed");
        close(server_socket);
        return -1;
    }

    cout << "Server listening on port: "<< port <<endl;


    std::vector<std::thread> threads;
    Thread_pool th_pool;

    for (int i = 0; i < thread_pool_size; i++)
    {
        threads.push_back(std::thread(&Thread_pool::infinite_loop_func, &th_pool));
    }
    
    cout<< "Thread-pool created with number of threads: "<< thread_pool_size <<endl;
    
    std::thread q_len_logging_th(&Thread_pool::logQueueLength,&th_pool);
    q_len_logging_th.detach();
    
    while (true)
    {
        // gets new client socket upon 'Accept'
        int client_socket = getClientSocket(server_socket);
        if (client_socket < 0)
            continue;

        if(th_pool.getCurrQueueLen() < th_pool.Q_MAX_SIZE)
            th_pool.push(client_socket);
        else
            close(client_socket);
            //shutdown(client_socket, SHUT_RDWR);
    }

    close(server_socket);

    return 0;
}