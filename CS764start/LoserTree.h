#include "Iterator.h"

struct TreeNode{
	Item* _value;
	size_t _run_index;
	size_t _element_index;
	TreeNode(Item item, uint32_t run_index, uint32_t element_index);
	TreeNode();
	bool operator < (const TreeNode & other) const;
    bool operator > (const TreeNode & other) const;
	~TreeNode();
};

class LoserTree {
private:
    uint32_t _run_num;
    TreeNode** tree;

public:
    LoserTree(uint32_t leaf_num);

    ~LoserTree();

    bool empty();

    TreeNode* top();

    void push(Item item, uint32_t run_index, uint32_t element_index);

    void adjust(uint32_t run_index);
};