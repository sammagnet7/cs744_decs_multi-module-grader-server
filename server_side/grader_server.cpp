
#include "thread_pool.hpp"

#include <iostream>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <vector>
#include <stdint.h>
#include<thread>
#include <unistd.h>

using namespace std;




//<<<<<<<<<<<<<<<===================== main method below ======================>>>>>>>>>>>>>>>>
//
//<<<<<<<<<<<<<<<===============================================================>>>>>>>>>>>>>>>>

int main(int argc, char *argv[])
{
    // Describe Usage
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <thread_pool_size>" << endl;
        return 1;
    }
    // Extracting from cmd args
    int thread_pool_size = atoi(argv[1]);

    cout << "Grading server started processing.." <<endl;


    std::vector<std::thread> threads;
    Thread_pool th_pool;

    for (int i = 0; i < thread_pool_size; i++)
    {
        threads.push_back(std::thread(&Thread_pool::infinite_grading_loop_func, &th_pool));
    }
    
    cout<< "Thread-pool created with number of threads: "<< thread_pool_size <<endl;

    sleep(10);
    
    // Main thread waits for each thread to finish
    for (int i = 0; i < thread_pool_size; i++)
    {
        threads[i].join();
    }


    return 0;
}