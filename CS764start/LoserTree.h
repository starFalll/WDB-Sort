#pragma once

#include "Iterator.h"
#include "ovc.h"

struct TreeNode{
    // data record pointer
	Item* _value;
    // index of data's run
	int32_t _run_index;
    // index of data in the run
	int32_t _element_index;
    // offset value code
    uint32_t _offset_value_code; 
	TreeNode(Item* item, int32_t run_index, int32_t element_index);
	TreeNode();
	bool operator < (const TreeNode & other) const;
    bool operator > (const TreeNode & other) const;
	~TreeNode();
};

class LoserTree {
private:
    // number of merge runs
    int32_t _leaf_num;
    // loser tree (implemented by array)
    TreeNode** _tree;
    // max item
    Item* ITEM_MIN;
    // min item
    Item* ITEM_MAX;

public:
    LoserTree(int32_t leaf_num, RowSize row_size);

    ~LoserTree();

    bool empty();

    TreeNode* top();

    void push(Item* item, int32_t run_index, int32_t element_index, std::string* baseStr);

    void adjust(int32_t run_index);

    void reset(int32_t num_of_reset_nodes, Item* value);

    Item* getMinItem();

    Item* getMaxItem();

};