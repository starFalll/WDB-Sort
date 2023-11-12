
#ifndef OCV_H
#define OCV_H

#include <cstdint>

class ovc {
public:
    uint32_t calculate_offset_value_code(const std::string& base, const std::string& compare);
};

#endif
