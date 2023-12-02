#pragma once

#include "Iterator.h"
#include "ovc.h"

struct TreeNode{
    // data record pointer
	const Item* _value;
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

public:
    LoserTree(int32_t leaf_num);

    ~LoserTree();

    bool empty();

    TreeNode* top();

    //todo temp
    std::string getvalue(int i);
    void push(const Item* item, int32_t run_index, int32_t element_index, const FieldType* baseStr);

    void adjust(int32_t run_index, const FieldType* base_str_ptr);

    void reset(int32_t num_of_reset_nodes, const Item* value);
};