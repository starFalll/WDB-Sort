// #include <fcntl.h>
// #include <unistd.h>
#include <aio.h>
#include "Iterator.h"

class File{
private:
    const char* _file_path;
    std::fstream _file_stream;
    // file size
    long long _max_byte;
    // current size
    long long _cur_byte;

public:
    File(const char* path, long long _max_byte);
    ~File();

    void write(const char* data, int32_t length);

    bool isFull();
};