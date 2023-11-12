#include "ovc.h"
#include<iostream>
uint32_t calculate_offset_value_code(const std::string& base, const std::string& compare) {
    //TODO: check if string have same length
    // check the max of two string length
    int max_length = std::max(base.length(), compare.length());
    //copy
    std::string base_str = base;
    std::string compare_str = compare;

    //from left insert 0
    base_str.insert(0, max_length - base.length(), '0');
    compare_str.insert(0, max_length - compare.length(), '0');

    // compare from left to right
    for (int i = 0; i < max_length; ++i) {
        //first different char
        if (base_str[i] != compare_str[i]) {
            int offset = max_length - i - 1; 
            int offset_Data = compare_str[i] - base_str[i];
            return offset * 100 + offset_Data;
        }
    }

    return 0; // equal
}
base:    0425
compare: 3582 
compare: 3461 
         0123
403: 3@0

