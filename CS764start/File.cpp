#include "File.h"

File::File(const char* path, unsigned long long max_byte, int32_t block_size, std::ios::openmode m, FileType type) : 
    _file_path(path), _max_byte(max_byte), _block_size(block_size), _run_num(0), _type(type){
        // open file
    _file_stream.open(_file_path, std::ios::in | std::ios::binary | m);

    if (!_file_stream.is_open()) {
        // extract directory
        std::regex re("(.*/)[^/]+");
        std::cmatch cm;
        std::regex_match(_file_path, cm, re);
        // create directory and file
        int status = std::system(("[ -d '" + cm[1].str() + "' ]").c_str());
        if(status != 0){
            status = std::system(("mkdir " + cm[1].str() + "&& touch " + std::string(_file_path)).c_str());
        }else{
            status = std::system(("touch " + std::string(_file_path)).c_str());
        }
        // open file
        _file_stream.open(_file_path, std::ios::in | std::ios::binary | m);

        if(!_file_stream.is_open() || status != 0){
            std::cerr << "Error opening file: " << _file_path << std::endl;
        }
    }
}


File::File(const char* path, FileType type) : _file_path(path), _type(type)
{
    // open file
    _file_stream.open(_file_path, std::ios::in | std::ios::binary);
    if (!_file_stream.is_open()) {
        std::cerr << "Error reading file: " << _file_path << std::endl;
    }
}

File::File(const char* path, unsigned long long max_byte, int32_t block_size, FileType type) : 
    _file_path(path), _max_byte(max_byte), _block_size(block_size), _run_num(0), _type(type)
{
    // open file
    _file_stream.open(_file_path, std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);

    if (!_file_stream.is_open()) {
        // extract directory
        std::regex re("(.*/)[^/]+");
        std::cmatch cm;
        std::regex_match(_file_path, cm, re);
        // create directory and file
        int status = std::system(("[ -d '" + cm[1].str() + "' ]").c_str());
        if(status != 0){
            status = std::system(("mkdir " + cm[1].str() + "&& touch " + std::string(_file_path)).c_str());
        }else{
            status = std::system(("touch " + std::string(_file_path)).c_str());
        }
        // open file
        _file_stream.open(_file_path, std::ios::out | std::ios::in | std::ios::trunc | std::ios::binary);

        if(!_file_stream.is_open() || status != 0){
            std::cerr << "Error opening file: " << _file_path << std::endl;
        }
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
        // _cur_byte += length;

    }else{
        printf("Fail to write SSD.");
    }
}

char* File::read(GroupCount group_num, RowSize row_size, RowCount each_group_row_count, BatchSize batch_size , 
    uint32_t group_offset, BatchSize* read_size){
    if(_file_stream.is_open()){
        // bytes need reading
        int need_reading = batch_size * row_size ;
        char* buffer = new char[need_reading];
        memset(buffer, 0, need_reading);

        // set read start from index
        std::streampos start_index = group_num  * each_group_row_count * row_size + group_offset * row_size; 
        _file_stream.seekg(start_index);

        // read
        _file_stream.read(buffer, need_reading);
        if (_file_stream.eof()) {
            *read_size = _file_stream.gcount();
        }
        else {
            *read_size = need_reading;
        }
        *read_size /= row_size;
        return buffer;
    } else{
        printf("Fail to read disk.");
    }
    return nullptr;
}

char* File::read(int32_t start, int32_t length){
    if(_file_stream.is_open()){
        char* buffer = new char[length];
        memset(buffer, 0, length);
        _file_stream.seekg(start, std::ios::beg);
        // read
        _file_stream.read(buffer, length);

        return buffer;
    }else{
        printf("Fail to read.");
    }
    return nullptr;
}

bool File::isFull(){
    return getCurByte() >= _max_byte; 
}

int32_t File::getBlockSize(){
    return _block_size;
}

void File::recordRunSize(int32_t size) {
    _group_lens[_run_num-1] += size;
}

void File::addRunNum(){
    _group_lens.push_back(0);
    _run_num++;
}

unsigned long long File::getCurByte(){
    _file_stream.seekg(0, std::ios::end);
    return _file_stream.tellg();
}
