#include "timestamp.hpp"

#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>

Timestamp::Timestamp(char *format)
{
    this->format = new char[strlen(format) + 1];
    strcpy(this->format, format);
}

Timestamp::~Timestamp()
{
    if (format)
        delete[] format;
}

void Timestamp::SetTimestamp(int64_t timestamp_us)
{
    char buffer[20];
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm now_tm = {};
    localtime_r(&now_time_t, &now_tm);

    std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &now_tm);
    SetText(buffer);
}