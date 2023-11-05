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

void Thread_pool::countQueueLength(){
    
    std::string directoryPath = "temp_files";
    std::string filePath = "temp_files/avgQ.log";

    // Check if the directory exists or create it if it doesn't
    if (!std::filesystem::exists(directoryPath)) {
        std::filesystem::create_directory(directoryPath);
    }

    // Open the file inside the directory
    std::ofstream file(filePath, std::ios::out);

    if (file.is_open()) {
        std::string q_size_output = "";
        
        while (true) {
            auto ms = std::chrono::high_resolution_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(ms);
            char time[100]; // Buffer to hold the formatted date and time
            std::strftime(time, sizeof(time), "%H:%M:%S", std::localtime(&now_c));

            int q_length = task_queue.size();

            string t(time);
            
            std::string str = t + " " + std::to_string(q_length);

            file << str << std::endl;
            // std::cout << str << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }

        file.close(); // File closed after the loop finishes writing data
    } else {
        std::cout << "Error opening the file." << std::endl;
    }
}