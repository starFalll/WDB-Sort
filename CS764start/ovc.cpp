#include "ovc.h"

/*
base:    0425
compare: 3582 
compare: 3461 
403: 3@0
*/
uint32_t CalculateOffsetValueCode(std::string* base, std::string* compare) {
    //TODO: check if string have same length
    // check the max of two string length
    int max_length = std::max(base->length(), compare->length());

    // compare from left to right
    for (int i = 0; i < max_length; ++i) {
        //first different char
        if (base->at(i) != compare->at(i)) {
            int offset = max_length - i;
            // Set the loser value to the value of the current number of digits
            int offset_Data = compare->at(i) - '0';
            return offset * 100 + offset_Data;
        }
    }

    return 0; // equal
}



