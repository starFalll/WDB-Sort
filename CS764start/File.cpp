#include "File.h"

File::File(const char* path, unsigned long long max_byte, int32_t block_size) : 
    _file_path(path), _max_byte(max_byte), _cur_byte(0), _block_size(block_size)
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

long File::getFileSize(){
    struct stat statbuf;
    stat(_file_path , &statbuf);
    return statbuf.st_size;
}

void File::setReadPosition(int _eSize, int group_row){
    //number of groups in each file
    int groupNum = ceil(getFileSize()/ (_eSize * group_row));
    int* _read_position = new int[groupNum];
}

std::vector<Item>* File::getBatchRecords(int _eSize, int batch_size, int row_offset){
    std::vector<Item>* one_batch;
    if(_file_stream.is_open()){
        //jump to exact position
        _file_stream.seekg(row_offset*_eSize*3,std::ios::beg);
        // read one record
        for(int i = 1 ; i<= batch_size; i++){
            StringFieldType incl;
	        StringFieldType mem;
	        StringFieldType mgmt;
            char WordBuffer[_eSize*3];
            _file_stream.read(WordBuffer, _eSize*3);
            for(int tempIndex = 0;tempIndex<_eSize *3;tempIndex++){
                if (tempIndex <tempIndex<_eSize){incl.push_back(WordBuffer[tempIndex]);}
                if (tempIndex <tempIndex<_eSize*2){mem.push_back(WordBuffer[tempIndex]);}
                if (tempIndex <tempIndex<_eSize*3){mgmt.push_back(WordBuffer[tempIndex]);}
		    }
            one_batch->push_back(Item(incl,mem,mgmt));
            //read next record
		    _file_stream.seekg(_eSize * 3,std::ios::beg);
        }
    }else{
        printf("Fail to write file.");
    }
    return one_batch;
}

bool File::isFull(){
    return _cur_byte >= _max_byte; 
}

int32_t File::getBlockSize(){
    return _block_size;
}
