#include "File.h"

File::File(const char* path, long long max_byte) : 
    _file_path(path), _max_byte(max_byte), _cur_byte(0)
{
    // open file
    _file_stream.open(_file_path, std::ios::in | std::ios::out);

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

bool File::isFull(){
    return _cur_byte >= _max_byte; 
}