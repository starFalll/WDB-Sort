#include "SharedBuffer.h"

SharedBuffer::SharedBuffer(int32_t buffer_capacity) : 
    _buffer_capacity(buffer_capacity), _front(0), _rear(0), _finish(false)
{
    // init buffer
    _buffer = new Item[_buffer_capacity];
}

SharedBuffer::~SharedBuffer(){
    delete _buffer;
}

void SharedBuffer::produce(const Item& item, bool finish){
    // check if current produce slot valid
    std::unique_lock<std::mutex> lock(_mtx);
    _not_full_cv.wait(lock, [this]{ return !isBufferFull(); });

    // write buffer
    _buffer[_rear] = item;
    _rear = (_rear + 1) % _buffer_capacity;

    _finish = finish;

    _not_empty_cv.notify_all();
    lock.unlock();
}

void SharedBuffer::consume(File* file){
    int32_t block_size = file->getBlockSize();

    std::unique_lock<std::mutex> lock(_mtx);
    _not_empty_cv.wait(lock, [this, block_size]{ return !isBufferBigEnoughToConsume(block_size) || _finish; });
    
    // convert the number of bytes to the number of elements
    int32_t item_num = block_size / sizeof(Item); // todo: 获取item的实际大小
    if(_finish){
        item_num = _rear + _buffer_capacity - _front;
    }
    // check data continuity
    if(_front + item_num <= _buffer_capacity){
        // write
        file->write((char*)&(_buffer[_front]), item_num * sizeof(Item));
    }else{
        // first write
        int32_t tmp_num = _buffer_capacity - _front;
        file->write((char*)&(_buffer[_front]), tmp_num * sizeof(Item));
        // second write
        item_num -= tmp_num;
        file->write((char*)&(_buffer[0]), item_num * sizeof(Item));
    }
    // update front
    _front = (_front + item_num) % _buffer_capacity;

    _not_full_cv.notify_all();
    lock.unlock();
}

// todo: 回调函数，更新状态数组，要加锁

void SharedBuffer::cyclicalConsume(File* SSD, File* HDD){
    int32_t count=0;
    do{
        // sleep 0.1ms
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        count++;
        // 0.1ms = 100us
        if(!SSD->isFull()){
            std::thread ssd_consume([this, &SSD](){this->consume(SSD);});
        }
        if(count % 100 == 0){
            std::thread hdd_consume([this, &HDD](){this->consume(HDD);});
            count = 0;
        }
        
        // beacuse of mutex, we dont need to wait the thread finish
    } while (!isBufferEmpty());
}

bool SharedBuffer::isBufferEmpty(){
    return _front == _rear && _finish;
}

bool SharedBuffer::isBufferFull(){
    return _front == (_rear + 1) % _buffer_capacity;
    // todo: 判断状态数组
}

bool SharedBuffer::isBufferBigEnoughToConsume(int32_t length){
    return _rear + _buffer_capacity - _front >= length;
    // todo：判断状态数组
}

// todo
// sort 改 finish