#include "File.h"
#include <future>

class ProducerConsumer {
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
    //
    int32_t _ssd_consume_length;
    int32_t _hdd_consume_length;

public:
    ProducerConsumer(int32_t buffer_capacity);
    ~ProducerConsumer();

    void produce(const Item& item);

    void consume(File* file, int32_t block_size);

    void cyclicalConsume(File* SSD, File* HDD);

    bool isBufferEmpty();

    bool isBufferFull();

    bool isBufferBigEnoughToConsume(int32_t length);
};