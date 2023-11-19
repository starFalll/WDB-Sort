#include "File.h"
#include <future>

class SharedBuffer {
private:
    // buffer
    Item* _buffer;
    const int32_t _buffer_capacity;
    int32_t _front;
    int32_t _rear;
    //
    std::mutex _mtx;
    std::condition_variable _not_full_cv;
    std::condition_variable _not_empty_cv;
    bool _finish;
    //
    //todo：添加状态数组

public:
    SharedBuffer(int32_t buffer_capacity);
    ~SharedBuffer();

    void produce(const Item& item, bool finish);

    void consume(File* file);

    void cyclicalConsume(File* SSD, File* HDD);

    bool isBufferEmpty();

    bool isBufferFull();

    bool isBufferBigEnoughToConsume(int32_t length);
};