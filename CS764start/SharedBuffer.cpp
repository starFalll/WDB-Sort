#include "SharedBuffer.h"
#include <thread>

SharedBuffer::SharedBuffer(int32_t buffer_capacity, RowSize row_size) : 
    _buffer_capacity(buffer_capacity), _front(0), _rear(0), _finish(false), _row_size(row_size)
{
    // init buffer
    _buffer = new char[_buffer_capacity];
    _total_write_size = 0;
    _total_read_size = 0;
}

SharedBuffer::~SharedBuffer(){
    delete [] _buffer;
}

void SharedBuffer::produce(const Item& item, bool finish){

    std::unique_lock<std::mutex> lock(_mtx);
    _not_full_cv.wait(lock, [this]{ return !isBufferFull(); });

    // write buffer
    for(int i=0;i<3;i++){
        int32_t str_length = (i==2) ? _row_size-2*(_row_size/3) : _row_size/3;
        if(_buffer_capacity - _rear >= str_length){
            // strcpy(&(_buffer[_rear]), item.fields[i]);
            memcpy(&(_buffer[_rear]), item.fields[i], str_length);
            // printf("test1\n");
            printf("cur:%s\n", &(_buffer[_rear]));
            _total_read_size += str_length;
        }else{
            int32_t tmp_length = _buffer_capacity - _rear;
            printf("change size1:%d\n", tmp_length);
            memcpy(&(_buffer[_rear]), item.fields[i], tmp_length);
            _total_read_size += tmp_length;
            // strcpy(&(_buffer[_rear]), tmp_str.substr(0, tmp_length).c_str());
            std::string tmp_str(item.fields[i]);
            int32_t start_idx = tmp_length;
            tmp_length = str_length - tmp_length;
            memcpy(&(_buffer[0]), tmp_str.substr(start_idx, tmp_length-1).c_str(), tmp_length);
            _total_read_size += tmp_length;
            printf("change size2:%d\n", tmp_length);
            // strcpy(&(_buffer[0]), );
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
    int32_t add_size = 0;
    // write file
    if(block_size != 0){
        // check data continuity
        if(_front + block_size <= _buffer_capacity){
            // write
            file->write((char*)&(_buffer[_front]), block_size);
            _total_write_size += block_size;
            add_size = block_size;
            printf("write1:%s\n", (char*)&(_buffer[_front]));
        }else{
            // first write
            int32_t tmp_num = _buffer_capacity - _front;
            file->write((char*)&(_buffer[_front]), tmp_num);
            _total_write_size += tmp_num;
            add_size += tmp_num;
            // second write
            tmp_num = block_size - tmp_num;
            file->write((char*)&(_buffer[0]), tmp_num);
            _total_write_size += tmp_num;
            add_size += tmp_num;
            printf("write2:%s\n", (char*)&(_buffer[_front]));
        }
        // update front
        _front = (_front + block_size) % _buffer_capacity;
    }
    file->recordRunSize(add_size);
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
    traceprintf ("SharedBuffer read %llu write %lu\n", _total_read_size,
			(unsigned long) (_total_write_size));
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

bool SharedBuffer::isBufferFull(){
    return getAvailableSpace() <= _row_size;
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