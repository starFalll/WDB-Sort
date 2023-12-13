#pragma once
#include "defs.h"
#include <vector>

enum FileType {
    SSD = 0,
    HDD = 1
};

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

    FileType _type;
    RowSize _row_size;
 
    std::vector<int32_t> _group_lens;

public:
    File(const char* path, unsigned long long _max_byte, int32_t block_size, std::ios::openmode m, FileType type, RowSize row_size);
    File(const char* path, FileType type, RowSize row_size, int32_t block_size); //open (only in)
    File(const char* path, unsigned long long _max_byte, int32_t block_size, FileType type, RowSize row_size); //open and clear content
    ~File();

    // write file
    void write(const char* data, int32_t length);

    //read
    char* read(GroupCount group_num, RowSize row_size, std::vector<int>& each_group_row, BatchSize batch_size , uint32_t group_offset, BatchSize* read_size);

    char* read(uint64_t start, int32_t length, int32_t* read_size);

    // check if file is full(important for SSD)
    bool isFull();

    // get file bandwidth
    int32_t getBlockSize();

    void recordRunSize(int32_t size);
    
    // add number of runs
    void addRunNum();

    FileType getType() {
        return _type;
    }

    int32_t getRunNum() {
        return _run_num;
    }

    unsigned long long getCurByte() {
        return _cur_byte;
    }

    std::vector<int32_t> getGroupLens() {
        return _group_lens; 
    }
};