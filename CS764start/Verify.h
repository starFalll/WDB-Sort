#pragma once
# include "File.h"

struct Bucket{
    // char array
    char* _bucket;
    // array index
    int _bucket_index;

    Bucket(int bucket_capacity);
    ~Bucket();
};

class Verify{
private:
    // input
    File* _input_file; 
    // output
    File* _output_file;
    // row size
    int _row_size;
    // bucket capacity
    int _bucket_capacity;
    // file size
    unsigned long long _file_size;
    // bucket size
    int _bucket_num;
    //
    int _batch_size;
    // memory hashtable
    Bucket** _hash_table;
    // record each bucket's size
    int* _input_bucket_size;
    int* _output_bucket_size;

public:
    Verify();

    Verify(int row_size, unsigned long long file_size, unsigned long long memory_size);

    ~Verify();

    int hash(std::string record);

    void write_bucket(char* data, int length, int bucket_id, std::string dir_path);

    std::string read_bucket(int bucket_id, std::string dir_path, bool is_output);

    void create_hash_table(File* file, std::string dir_path, bool& order_status, bool is_output);

    void verify();
};