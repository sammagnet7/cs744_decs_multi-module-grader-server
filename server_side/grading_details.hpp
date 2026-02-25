
#pragma once

#include <string>
#include <chrono>

//This holds the structure of the database enttry which in turn holds all the info regarding submission request
struct GradingDetails {
    std::string trace_id;
    std::string progress_status;
    std::string submitted_file;
    std::string lastupdated;
    std::string grading_status;
    std::string grading_output;
};