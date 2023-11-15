
#pragma once

#include <string>
#include <chrono>

struct GradingDetails {
    std::string trace_id;
    std::string progress_status;
    std::string submitted_file;
    std::string lastupdated;
    std::string grading_status;
    std::string grading_output;
};