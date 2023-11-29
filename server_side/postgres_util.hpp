#pragma once

#include <iostream>
#include <thread>
#include <string>

#include "lib/pg/include/pqxx/pqxx"
#include "grading_details.hpp"
#include "fileio.hpp"

using namespace std;
using namespace pqxx;

class Postgres_util
{
public:
    void insertGradingDetails(const GradingDetails &details);
    GradingDetails retrieveGradingDetails(string trace_id);
    void updateGradingDetails(const GradingDetails &details);

private:
    static const std::string conn_addr_pg;

    static pqxx::connection dbconnect();
    static void printGradingDetails(const GradingDetails& details);   
};
