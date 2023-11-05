#pragma once
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>
#include <filesystem>

class Thread_pool
{

public:

    Thread_pool();
    ~Thread_pool();

    int count = 0;
    int q_length = 0;
    void push(int func);
    //void done();
    void infinite_loop_func();
    void countQueueLength();

    
private:
    std::queue<int> task_queue;
    std::mutex queue_mutex;
    std::condition_variable mutex_condition;
    std::atomic<bool> server_live;
};
