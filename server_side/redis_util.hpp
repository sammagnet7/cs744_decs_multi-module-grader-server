#pragma once
#include "lib/redis/include/sw/redis++/redis++.h"
# include <iostream>
#include<thread>
#include<string>

using namespace sw::redis;

class Redis_util
{

public:
    void ping();
    void pushBack( std::string traceId);
    std::string pullFront();

private:
    static auto connect();
    static const std::string conn_addr_redis;
    
};