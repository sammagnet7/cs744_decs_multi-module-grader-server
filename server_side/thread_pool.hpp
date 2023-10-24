#pragma once
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>

class Thread_pool
{

public:

    Thread_pool();
    ~Thread_pool();

    void push(int func);
    //void done();
    void infinite_loop_func();

    
private:
    std::queue<int> task_queue;
    std::mutex queue_mutex;
    std::condition_variable mutex_condition;
    std::atomic<bool> server_live;

};
