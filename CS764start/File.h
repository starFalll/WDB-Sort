// #include <fcntl.h>
// #include <unistd.h>
#include <thread>
#include <unistd.h>
#include "Iterator.h"
#include <sys/stat.h>
#include <math.h>

class File{
private:
    const char* _file_path;
    std::fstream _file_stream;
    // file size
    unsigned long long _max_byte;
    // current size
    unsigned long long _cur_byte;
    // block size (one access)
    int32_t _block_size;
    //read position
    int * _read_position;

public:
    File(const char* path, unsigned long long _max_byte, int32_t block_size);
    ~File();

    void write(const char* data, int32_t length);

    bool isFull();

    //status of data being read
    void setReadPosition(int _eSize, int group_row);
    std::vector<Item>* getBatchRecords(int _eSize, int batch_size, int row_offset);
    int32_t getBlockSize();
    long getFileSize();
};