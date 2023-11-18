#include "SharedBuffer.h"

ProducerConsumer::ProducerConsumer(int32_t buffer_capacity) : 
    _buffer_capacity(buffer_capacity), _front(0), _rear(0)
{
    // init buffer
    _buffer = new Item[_buffer_capacity];
}

ProducerConsumer::~ProducerConsumer(){
    delete _buffer;
}

void ProducerConsumer::produce(const Item& item){
    // check if current produce slot valid
    std::unique_lock<std::mutex> lock(_mtx);
    _not_full_cv.wait(lock, [this]{ return !isBufferFull(); });

    // write buffer
    _buffer[_rear] = item;
    _rear = (_rear + 1) % _buffer_capacity;

    lock.unlock();
    _not_empty_cv.notify_all();
}

void ProducerConsumer::consume(File* file){
    std::unique_lock<std::mutex> lock(_mtx);
    _not_empty_cv.wait(lock, [this](int32_t block_size){ return !isBufferBigEnoughToConsume(block_size); });
    
    // convert the number of bytes to the number of elements
    int32_t block_size = file->getBlockSize();
    int32_t item_num = block_size / sizeof(Item);
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

    lock.unlock();
    _not_full_cv.notify_all();
}

void ProducerConsumer::cyclicalConsume(File* SSD, File* HDD){
    int32_t count=0;
    do{
        // sleep 0.1ms
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        count++;
        // 0.1ms = 100us
        if(!SSD->isFull()){
            std::thread ssd_consume(consume, SSD);
        }
        if(count % 100 == 0){
            std::thread hdd_consume(consume, HDD);
            count = 0;
        }
        
        // beacuse of mutex, we dont need to wait the thread finish
    } while (!isBufferEmpty());
}

bool ProducerConsumer::isBufferEmpty(){
    return _front == _rear;
}

bool ProducerConsumer::isBufferFull(){
    return _front == (_rear + 1) % _buffer_capacity;
}

bool ProducerConsumer::isBufferBigEnoughToConsume(int32_t length){
    return _rear + _buffer_capacity - _front >= length;
}

// 最后一小块