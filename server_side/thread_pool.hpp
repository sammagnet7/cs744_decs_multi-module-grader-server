#pragma once
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <cassert>
#include <filesystem>
#include <random>

#include "fileio.hpp"

class Thread_pool
{

public:
    int Q_MAX_SIZE=4000;

    Thread_pool();
    ~Thread_pool();

    void push(int func);
    void infinite_submission_loop_func();
    void infinite_grading_loop_func();
    void infinite_statusCheck_loop_func();
    void logSharedQueueLength();
    void logServiceTime(long serviceTime);
    int getLocalQueueLen();

    
private:
    std::queue<int> task_queue;
    std::mutex queue_mutex;
    std::condition_variable mutex_condition;
    std::atomic<bool> server_live;
    static long long getUniqueId();
};
