#include "thread_pool.hpp"
#include "gradingserver_worker.hpp"

Thread_pool::Thread_pool() : task_queue(), queue_mutex(), mutex_condition(), server_live(true)
{
}

Thread_pool::~Thread_pool()
{
}

void Thread_pool::push(int sock)
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    task_queue.push(sock);
    lock.unlock();
    mutex_condition.notify_one();
}
/**
void Thread_pool::done()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    server_live = false;
    lock.unlock();
    mutex_condition.notify_all();
}*/

void Thread_pool::infinite_loop_func()
{
    int sockfd;
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this]() {return !task_queue.empty(); });
            if (task_queue.empty())
            {
                //lock will be release automatically.
                lock.unlock();
                continue;
            }
            sockfd = task_queue.front();
            task_queue.pop();
            //release the lock
            lock.unlock();
            worker_handler(sockfd);
        }
        
    }
}