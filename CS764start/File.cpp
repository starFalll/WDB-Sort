#include "File.h"

File::File(const char* path, unsigned long long max_byte, int32_t block_size) : 
    _file_path(path), _max_byte(max_byte), _cur_byte(0), _block_size(block_size), _run_num(0)
{
    // open file
    _file_stream.open(_file_path, std::ios::in | std::ios::out | std::ios::binary);

    if (!_file_stream.is_open()) {
        std::cerr << "Error opening file: " << _file_path << std::endl;
    }
}

File::~File() {
    if (_file_stream.is_open()) {
        _file_stream.close();
    }
}

void File::write(const char* data, int32_t length){
    if(_file_stream.is_open()){
        // write
        _file_stream.write(data, length);
        // record file size
        _cur_byte += length;

    }else{
        printf("Fail to write SSD.");
    }
}

char* File::read(GroupCount group_num , RowSize row_size, RowCount each_group_row_count, BatchSize batch_size){
    if(_file_stream.is_open()){
        // bytes need reading
        int need_reading = batch_size * row_size ;
        char* buffer = new char[need_reading];

        // set read start from index
        std::streampos start_index = group_num  * each_group_row_count * row_size; 
        _file_stream.seekg(start_index);

        // read
        _file_stream.read(buffer, need_reading);
        return buffer;
    } else{
        printf("Fail to read disk.");
    }
    return nullptr;
}

bool File::isFull(){
    return _cur_byte >= _max_byte; 
}

int32_t File::getBlockSize(){
    return _block_size;
}

void File::addRunNum(){
    _run_num++;
}