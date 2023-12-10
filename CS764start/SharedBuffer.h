#pragma once

#include "File.h"
#include "Item.h"
#include <future>

class SharedBuffer {
private:
    // buffer
    char* _buffer;
    const int32_t _buffer_capacity;
    int32_t _front;
    int32_t _rear;
    uint64_t _total_write_size;
    uint64_t _total_read_size;
    // mutex
    std::mutex _mtx;
    std::condition_variable _not_full_cv;
    std::condition_variable _not_empty_cv;
    // merge finish symbol
    bool _finish;
    //
    RowSize _row_size;

public:
    SharedBuffer(int32_t buffer_capacity, RowSize row_size);
    ~SharedBuffer();

    void produce(const Item& item, bool finish);

    void consume(File* file);

    void cyclicalConsume(File* SSD, File* HDD);
    void resConsume(File* RES_HDD);

    bool isBufferEmpty();

    bool isBufferFull();

    bool isBufferBigEnoughToConsume(int32_t length);

    int32_t getValidDataLength();

    int32_t getAvailableSpace();

    void reset();
};