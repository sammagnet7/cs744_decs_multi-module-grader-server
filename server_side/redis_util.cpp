    #include "redis_util.hpp"

    const std::string Redis_util::conn_addr_redis = "tcp://10.157.3.213:6379/";

    auto Redis_util::connect()
    {
        auto redis = Redis(conn_addr_redis);
        return redis;
    }

    void Redis_util::ping()
    {
        auto redis = connect();

        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: " + redis.ping();

        std::cout << log << std::endl;
    }

    void Redis_util::pushBack(std::string traceId)
    {
        auto redis = connect();

        /*LOGGING*/
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Pushing TraceId: " + traceId;
        std::cout << log << std::endl;

        redis.lpush("jobs", traceId);
    }

    std::string Redis_util::pullFront()
    {
        std::string traceId = "";

        try
        {
            auto redis = connect();
            // Assuming blpop is used and returns OptionalStringPair
            auto result = redis.blpop("jobs", std::chrono::seconds(0));
            if (result)
            {
                // Extract data from OptionalStringPair
                auto key = result->first;
                auto value = result->second;
                traceId= result->second;
                std::cout << "Key: " << key << ", Value: " << value << std::endl;
            }
            else
            {
                std::cout << "No elements in the list." << std::endl;
            }

            /*LOGGING*/
            auto threadId = std::this_thread::get_id();
            std::stringstream ss;
            ss << threadId;
            std::string thId = ss.str();
            std::string log = "Thread Id: " + thId + " :: Popped TraceId: " + traceId;
            std::cout << log << std::endl;
        }
        catch (const sw::redis::ProtoError &e)
        {
            std::cerr << e.what() << std::endl;
        }
        return traceId;
    }