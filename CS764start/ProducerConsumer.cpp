#include "ProducerConsumer.h"

ProducerConsumer::ProducerConsumer(int32_t buffer_capacity) : 
    _buffer_capacity(buffer_capacity), _front(0), _rear(0),
    _ssd_consume_length(100 * 1024 * 1024 / 10000),
    _hdd_consume_length(100 * 1024 * 1024 / 100) {
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

void ProducerConsumer::consume(File* file, int32_t block_size){
    std::unique_lock<std::mutex> lock(_mtx);
    _not_empty_cv.wait(lock, [this](int32_t block_size){ return !isBufferBigEnoughToConsume(block_size); });
    
    file->write((char*)&(_buffer[_front]), block_size);
    _front = (_front + block_size) % _buffer_capacity;

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
            std::thread ssd_consume(consume, SSD, _ssd_consume_length);
        }
        if(count % 100 == 0){
            std::thread hdd_consume(consume, HDD, _hdd_consume_length);
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

// 正好rear小于front怎么读
// 最后一小块