#include "SharedBuffer.h"

SharedBuffer::SharedBuffer(int32_t buffer_capacity) : 
    _buffer_capacity(buffer_capacity), _front(0), _rear(0), _finish(false)
{
    // init buffer
    _buffer = new char[_buffer_capacity];
}

SharedBuffer::~SharedBuffer(){
    // delete _buffer;
}

void SharedBuffer::produce(const Item& item, bool finish){
    int32_t row_size=0;
    for(int i=0;i<3;i++){
        row_size += item.fields[i].size();
    }

    std::unique_lock<std::mutex> lock(_mtx);
    _not_full_cv.wait(lock, [this, row_size]{ return !isBufferFull(row_size); });

    // write buffer
    for(int i=0;i<3;i++){
        int32_t str_length = item.fields[i].size();
        if(_buffer_capacity - _rear >= str_length){
            strcpy(&(_buffer[_rear]), item.fields[i].c_str());
        }else{
            int32_t tmp_length = _buffer_capacity - _rear;
            strcpy(&(_buffer[_rear]), item.fields[i].substr(0, tmp_length).c_str());
            int32_t start_idx = tmp_length;
            tmp_length = str_length - tmp_length;
            strcpy(&(_buffer[0]), item.fields[i].substr(start_idx, tmp_length).c_str());
        }
        // update rear
        _rear = (_rear + str_length) % _buffer_capacity;
    }
    // _buffer[_rear] = item;
    // _rear = (_rear + 1) % _buffer_capacity;

    _finish = finish;

    _not_empty_cv.notify_all();
    lock.unlock();
}

void SharedBuffer::consume(File* file){
    int32_t block_size = file->getBlockSize();

    std::unique_lock<std::mutex> lock(_mtx);
    _not_empty_cv.wait(lock, [this, block_size]{ return isBufferBigEnoughToConsume(block_size) || _finish; });
    
    // convert the number of bytes to the number of elements
    // int32_t item_num = block_size / sizeof(Item); // todo: 获取item的实际大小
    if(_finish){
        block_size = getValidDataLength();
    }
    // write file
    if(block_size != 0){
        // check data continuity
        if(_front + block_size <= _buffer_capacity){
            // write
            std::cout<<_buffer<<std::endl;
            file->write((char*)&(_buffer[_front]), block_size);
        }else{
            // first write
            int32_t tmp_num = _buffer_capacity - _front;
            file->write((char*)&(_buffer[_front]), tmp_num);
            // second write
            tmp_num = block_size - tmp_num;
            file->write((char*)&(_buffer[0]), tmp_num);
        }
        // update front
        _front = (_front + block_size) % _buffer_capacity;
    }
    
    _not_full_cv.notify_all();
    lock.unlock();
}

// todo: 回调函数，更新状态数组，要加锁

void SharedBuffer::cyclicalConsume(File* SSD, File* HDD){
    int32_t count=0;
    // update file run number
    if(!SSD->isFull()){
        SSD->addRunNum();
    }
    HDD->addRunNum();
    // loop until merge finish
    while (!isBufferEmpty()) {
        // sleep 0.1ms
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        count++;
        // 0.1ms = 100us
        if(!SSD->isFull()){
            // ssd consume
            std::thread ssd_consume([this, &SSD](){this->consume(SSD);});
            // join thread
            ssd_consume.join();
        }
        if(count % 100 == 0){
            // hdd consume
            std::thread hdd_consume([this, &HDD](){this->consume(HDD);});
            count = 0;
            // join thread
            hdd_consume.join();
        }
    }
}

// only HDD
void SharedBuffer::resConsume(File* RES_HDD){
    int32_t count=0;
    // update file run number
    RES_HDD->addRunNum();
    // loop until merge finish
    while (!isBufferEmpty()) {
        // sleep 10ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        // hdd consume
        std::thread hdd_consume([this, &RES_HDD](){this->consume(RES_HDD);});
        count = 0;
        // join thread
        hdd_consume.join();
    }
}

bool SharedBuffer::isBufferEmpty(){
    return _front == _rear && _finish;
}

bool SharedBuffer::isBufferFull(int32_t row_size){
    return getAvailableSpace() < row_size;
    // todo: 判断状态数组
}

bool SharedBuffer::isBufferBigEnoughToConsume(int32_t length){
    return getValidDataLength() >= length;
    // todo：判断状态数组
}

int32_t SharedBuffer::getValidDataLength(){
    int32_t length = 0;
    if(_rear >= _front){
        length = (_rear - _front);
    }else{
        length = (_rear + _buffer_capacity - _front);
    }
    return length;
}

int32_t SharedBuffer::getAvailableSpace(){
    int32_t length = 0;
    if(_rear >= _front){
        length = _buffer_capacity - (_rear - _front);
    }else{
        length = _front - _rear;
    }
    return length;
}

void SharedBuffer::reset(){
    _front = 0;
    _rear = 0;
    _finish = false;
}