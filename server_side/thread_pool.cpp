#include "thread_pool.hpp"
#include "gradingserver_worker.hpp"

Thread_pool::Thread_pool() : task_queue(), queue_mutex(), mutex_condition(), server_live(true)
{
}

Thread_pool::~Thread_pool()
{
}

long long Thread_pool::getUniqueId()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long int> dist(0, std::numeric_limits<long long int>::max());
    long long id = dist(gen);
    return id;
}

void Thread_pool::push(int sock)
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    task_queue.push(sock);
    lock.unlock();
    mutex_condition.notify_one();
}

void Thread_pool::infinite_submission_loop_func()
{
    int sockfd;
    while (true)
    {

        std::unique_lock<std::mutex> lock(queue_mutex);
        mutex_condition.wait(lock, [this]()
                             { return !task_queue.empty(); });
        if (task_queue.empty())
        {
            // lock will be released automatically.
            lock.unlock();
            continue;
        }
        sockfd = task_queue.front();
        task_queue.pop();

        /*Assigns UNIQUE ID to each submission request*/
        long long traceId = getUniqueId();

        // release the lock
        lock.unlock();
        submission_worker_handler(sockfd, to_string(traceId));
    }
}

void Thread_pool::infinite_grading_loop_func()
{
    /* LOGGING */
    auto threadId = std::this_thread::get_id();
    std::stringstream ss;
    ss << threadId;
    std::string thId = ss.str();
    std::string log = "Thread Id: " + thId + " :: Grading Worker starts";
    std::cout << log << std::endl;

    while (true)
    {
        grader_worker_handler();
    }
}

void Thread_pool::infinite_statusCheck_loop_func()
{
    int sockfd;
    while (true)
    {

        std::unique_lock<std::mutex> lock(queue_mutex);
        mutex_condition.wait(lock, [this]()
                             { return !task_queue.empty(); });
        if (task_queue.empty())
        {
            // lock will be released automatically.
            lock.unlock();
            continue;
        }
        sockfd = task_queue.front();
        task_queue.pop();

        // release the lock
        lock.unlock();

        statusCheck_worker_handler(sockfd);
    }
}

int Thread_pool::getLocalQueueLen()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    int size = task_queue.size();
    lock.unlock();
    return size;
}

void Thread_pool::logSharedQueueLength()
{

    std::string directoryPath = "temp_files";
    std::string filePath = "temp_files/avgQ.log";

    // Check if the directory exists or create it if it doesn't
    if (!std::filesystem::exists(directoryPath))
    {
        std::filesystem::create_directory(directoryPath);
    }

    // Open the file inside the directory
    std::ofstream file(filePath, std::ios::out);

    if (file.is_open())
    {
        std::string q_size_output = "";

        while (true)
        {

            auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());

            char curr_time[100]; // Buffer to hold the formatted time
            std::strftime(curr_time, sizeof(curr_time), "%H:%M:%S", std::localtime(&timenow));
            
            long long q_length = getSharedQueueLength();
            if (q_length == 0){
                sleep(5);
                continue;
            }

            string curr_time_(curr_time);

            std::string q_line = curr_time_ + " " + std::to_string(q_length);

            file << q_line << std::endl;

            sleep(5);
        }

        file.close(); // File closed after the loop finishes writing data
    }
    else
    {
        std::cout << "Error opening the file." << std::endl;
    }
}