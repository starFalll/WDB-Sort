#pragma once
#include "defs.h"

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
    File(const char* path, unsigned long long _max_byte, int32_t block_size, std::ios::openmode m);
    File(const char* path); //open (only in)
    File(const char* path, unsigned long long _max_byte, int32_t block_size); //open and clear content
    ~File();

    // write file
    void write(const char* data, int32_t length);

    //read
    char* read(GroupCount group_num , RowSize row_size, RowCount each_group_row_count, BatchSize batch_size, uint32_t group_offset);

    char* read(int32_t start, int32_t length);

    // check if file is full(important for SSD)
    bool isFull();

    // get file bandwidth
    int32_t getBlockSize();
    
    // add number of runs
    void addRunNum();

    unsigned long long getCurByte();
};