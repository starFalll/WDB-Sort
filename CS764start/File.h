#pragma once

// #include <fcntl.h>
// #include <unistd.h>
#include <aio.h>
#include "Iterator.h"

typedef uint32_t GroupCount;
typedef uint32_t BatchSize;
class File{
private:
    // file path
    const char* _file_path;
    // file stream
    std::fstream _file_stream;
    // file size
    unsigned long long _max_byte;
    // current size
    unsigned long long _cur_byte;
    // block size (one access)
    int32_t _block_size;
    // number of runs
    int32_t _run_num;

public:
    File(const char* path, unsigned long long _max_byte, int32_t block_size);
    ~File();

    // write file
    void write(const char* data, int32_t length);

    //read
    char* read(GroupCount group_num ,RowSize row_size,RowCount each_group_row_count, BatchSize batch_size);

    // check if file is full(important for SSD)
    bool isFull();

    // get file bandwidth
    int32_t getBlockSize();
    
    // add number of runs
    void addRunNum();
};