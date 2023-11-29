#include "redis_util.hpp"

/**Modify connection string depending upon Redis server address*/
const std::string Redis_util::conn_addr_redis = "tcp://10.157.3.213:6379/";
/**Choose a suitable queue name for redis server*/
const std::string Redis_util::queue_name = "jobs";

//This function creates static connection object for connecting to Redis shared queue
auto Redis_util::connect()
{
    auto redis = Redis(conn_addr_redis);
    return redis;
}

//Utility function to ping Redis queue service
void Redis_util::ping()
{
    auto redis = connect();

    auto threadId = std::this_thread::get_id();
    std::stringstream ss;
    ss << threadId;
    std::string thId = ss.str();
    std::string log = "Thread Id: " + thId + " :: " + redis.ping();

    logMessageToFile(log);
}

//Utility function to push element into Redis queue
void Redis_util::pushBack(std::string traceId)
{
    try
    {
        auto redis = connect();

        redis.lpush(queue_name, traceId);

        /*LOGGING*/
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Pushed TraceId: " + traceId;
        logMessageToFile(log);
    }
    catch (const std::exception &e)
    {
        logMessageToFile( e.what() );
    }
}

//Utility function to pull element from Redis queue
std::string Redis_util::pullFront()
{
    std::string traceId = "";

    try
    {
        auto redis = connect();
        // Assuming blpop is used and returns OptionalStringPair
        auto result = redis.blpop(queue_name, std::chrono::seconds(0));
        if (result)
        {
            // Extract data from OptionalStringPair
            auto key = result->first;
            auto value = result->second;
            traceId = result->second;
            std::string log = "Key: "+ key + ", Value: " + value;
            logMessageToFile( log );
        }
        else
        {
            logMessageToFile( "No elements in the list."  );
        }

        /*LOGGING*/
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Popped TraceId: " + traceId;
       logMessageToFile( log );
    }
    catch (const sw::redis::ProtoError &e)
    {
        logMessageToFile( e.what() );
    }
    catch (const std::exception &e)
    {
        logMessageToFile( e.what() );
    }
    return traceId;
}

long long Redis_util::getLength()
{
    try
    {
        auto redis = connect();
        long long len = redis.llen(queue_name);
        return len;
    }
     catch (const std::exception &e)
    {
        logMessageToFile( e.what() );
    }
}

int Redis_util::getPos(std::string trace_id)
{

    try
    {   
        auto redis = connect();
        
        // Use the LPOS command to get the index of the first occurrence of the element in the list
        auto position = redis.command("LPOS", queue_name, trace_id);
        
        if (position && position->type == REDIS_REPLY_INTEGER)
        {
           
            // The element was found in the list
            return static_cast<int>(position->integer);
        }
        else
        {   
            // The element was not found in the list
            return -1;
        }
    }
    catch (const sw::redis::ProtoError &e)
    {
        
        logMessageToFile( e.what() );
    }
}