#pragma once
# include "File.h"

struct Bucket{
    // char array
    char* _bucket;
    // array index
    int _bucket_index;

    Bucket(int bucket_capacity);
    void rest_bucket();
};

class Verify{
private:
    // input
    File* _input_file; 
    // output
    File* _output_file;
    // 
    unsigned long long _file_size;
    //
    unsigned long long _mem_size;
    // row size
    int _row_size;
    // bucket capacity
    int _bucket_capacity;
    // bucket size
    int _bucket_num;
    // memory hashtable
    Bucket** _hash_table;

public:
    Verify();

    Verify(int row_size, unsigned long long file_size, unsigned long long memory_size);

    ~Verify();

    int hash(std::string record);

    void write_bucket(char* data, int length, int bucket_id, std::string dir_path);

    char* read_bucket(int bucket_id, std::string dir_path);

    void create_hash_table(File* file, int batch_size, std::string dir_path, bool& order_status, bool check_order = false);

    void reset_hashtable();

    void verify();
};