#include "File.h"

File::File(const char* path, unsigned long long max_byte, int32_t block_size) : 
    _file_path(path), _max_byte(max_byte), _cur_byte(0), _block_size(block_size), _run_num(0)
{
    // open file
    _file_stream.open(_file_path, std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);

    if (!_file_stream.is_open()) {
        // extract directory
        std::regex re("(.*/)[^/]+");
        std::cmatch cm;
        std::regex_match(_file_path, cm, re);
        // create directory and file
        int result = std::system(("mkdir " + cm[1].str() + "&& touch " + std::string(_file_path)).c_str());
        if(result != 0){
            std::cerr << "Error creating file: " << _file_path << std::endl;
        }
        // open file
        _file_stream.open(_file_path, std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);
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
        _file_stream.put('\n');
        // record file size
        _cur_byte += length;

    }else{
        printf("Fail to write SSD.");
    }
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
