#include "postgres_util.hpp"

const std::string Postgres_util::conn_addr_pg = "postgresql://cs744:cs744@10.157.3.213:5432/autograder?connect_timeout=10";

pqxx::connection Postgres_util::dbconnect()
{
    connection conn(conn_addr_pg);
    if (conn.is_open())
    {
        /* LOGGING */
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Opened database successfully: " + conn.dbname();
        std::cout << log << std::endl;
    }
    else
    {
        /* LOGGING */
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Can't open database";
        std::cout << log << std::endl;

        throw runtime_error("Failed to open database connection");
    }
    return conn;
}

void Postgres_util::insertGradingDetails(const GradingDetails &details)
{
    try
    {
        /* Create a transactional object. */
        pqxx::connection conn = dbconnect();
        pqxx::work W(conn);

        // Get the current time
        std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::ostringstream timestampStrStream;
        timestampStrStream << std::put_time(std::localtime(&timestamp), "%d-%m-%Y %H:%M:%S");
        std::string timestampStr = timestampStrStream.str();

        // Construct the SQL string using a string stream
        std::ostringstream sqlStream;
        sqlStream << "INSERT INTO grading_details (trace_id, progress_status, submitted_file, lastupdated, grading_status, grading_output) VALUES ("
                  << details.trace_id << ", '" << details.progress_status << "', '" << details.submitted_file << "', '"
                  << timestampStr << "', '" << details.grading_status << "', '" << details.grading_output << "');";
        std::string sql = sqlStream.str();
        // std::cout << sql << std::endl;

        /* Execute SQL query */
        W.exec(sql);
        W.commit();
        conn.close();

        /* LOGGING */
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Records inserted successfully where TraceId: " + details.trace_id;
        std::cout << log << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}

GradingDetails Postgres_util::retrieveGradingDetails(string trace_id)
{
    GradingDetails details;

    try
    {
        pqxx::connection conn = dbconnect();
        pqxx::nontransaction N(conn);

        // Execute SQL query to retrieve grading details for a specific trace_id
        string sql = "SELECT * FROM grading_details WHERE trace_id = " + trace_id + ";";
        result R(N.exec(sql));

        // Assume the first row is the result (adjust as needed)
        if (!R.empty())
        {
            const auto &row = R[0];
            details.trace_id = row["trace_id"].as<string>();
            details.progress_status = row["progress_status"].as<string>();
            details.submitted_file = row["submitted_file"].as<string>();
            details.lastupdated = row["lastupdated"].as<string>();
            details.grading_status = row["grading_status"].as<string>();
            details.grading_output = row["grading_output"].as<string>();
        }

        /* LOGGING */
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Records retrieved successfully where TraceId: " + details.trace_id;
        std::cout << log << std::endl;

        conn.close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return details;
}

void Postgres_util::updateGradingDetails(const GradingDetails &details)
{   
    try
    {
        /* Create a transactional object. */
        pqxx::connection conn = dbconnect();
        pqxx::work W(conn);

        // Get the current time
        std::time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::ostringstream timestampStrStream;
        timestampStrStream << std::put_time(std::localtime(&timestamp), "%d-%m-%Y %H:%M:%S");
        std::string timestampStr = timestampStrStream.str();

        // Construct the SQL string using a string stream
        std::ostringstream updateStream;
        updateStream << "UPDATE grading_details SET "
                     << "progress_status = '" << details.progress_status << "', "
                     << "submitted_file = '" << details.submitted_file << "', "
                     << "lastupdated = '" << timestampStr << "', "
                     << "grading_status = '" << details.grading_status << "', "
                     << "grading_output = '" << details.grading_output << "' "
                     << "WHERE trace_id = " << details.trace_id;

        std::string sql = updateStream.str();
        // std::cout << sql << std::endl;

        /* Execute SQL query */
        W.exec(sql);
        W.commit();
        conn.close();

        /* LOGGING */
        auto threadId = std::this_thread::get_id();
        std::stringstream ss;
        ss << threadId;
        std::string thId = ss.str();
        std::string log = "Thread Id: " + thId + " :: Records updated successfully where TraceId: " + details.trace_id;
        std::cout << log << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}