#include "ovc.h"

/*
base:    0425
compare: 3582 
compare: 3461 
403: 3@0
*/
uint32_t CalculateOffsetValueCode(const char* winner, const char* loser) {
    std::string base(winner);
    std::string compare(loser);
    // check the max of two string length
    int max_length = std::max(base.length(), compare.length());

    // compare from left to right
    for (int i = 0; i < max_length; ++i) {
        //first different char
        if (base[i] != compare[i]) {
            int offset = max_length - i;
            // Set the loser value to the value of the current number of digits
            int offset_Data = compare[i] - '0';
            return offset * 100 + offset_Data;
        }
    }

    return 0; // equal
}



