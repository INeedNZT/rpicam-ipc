#include "timestamp.hpp"

#include <ctime>
#include <chrono>
#include <sstream>

Timestamp::Timestamp(const char *format)
{
    this->format = new char[strlen(format) + 1];
    strcpy(this->format, format);
}

Timestamp::~Timestamp()
{
    if (format)
        delete[] format;
}

void Timestamp::SetTimestamp(int64_t timestamp_sec)
{
    using namespace std::chrono;
    
    time_point<system_clock, seconds> tp{seconds{timestamp_sec}};
    std::time_t t = system_clock::to_time_t(tp);
    std::tm now_tm = *std::localtime(&t);
    
    char buffer[128];
    std::strftime(buffer, sizeof(buffer), format, &now_tm);
    
    SetText(buffer);
}