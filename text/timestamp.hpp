#include "text.hpp"
#include <cstdint>

class Timestamp : public Text
{
public:
    Timestamp(const char *format);
    ~Timestamp();
    void SetTimestamp(int64_t timestamp_sec);
    
private:
    char *format;
};