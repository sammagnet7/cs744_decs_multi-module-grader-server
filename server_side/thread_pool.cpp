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

int Thread_pool::getCurrQueueLen(){
    std::unique_lock<std::mutex> lock(queue_mutex);
    int size= task_queue.size();
    lock.unlock();
    return size;
}

void Thread_pool::logQueueLength(){
    
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

            auto sys_time = std::chrono::high_resolution_clock::now();
            std::time_t sys_time_form = std::chrono::system_clock::to_time_t(sys_time);

            char curr_time[100]; // Buffer to hold the formatted time
            std::strftime(curr_time, sizeof(curr_time), "%H:%M:%S", std::localtime(&sys_time_form));

            int q_length = getCurrQueueLen();
            if(q_length==0)
                continue;

            string curr_time_(curr_time);
            
            std::string q_line = curr_time_ + " " + std::to_string(q_length);

            file << q_line << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        file.close(); // File closed after the loop finishes writing data
    } else {
        std::cout << "Error opening the file." << std::endl;
    }
}