#include "Iterator.h"

struct TreeNode{
	Item* _value;
	size_t _run_index;
	size_t _element_index;
	TreeNode(Item item, uint32_t run_index, uint32_t element_index);
	TreeNode();
	bool operator < (const TreeNode & other) const;
    bool operator > (const TreeNode & other) const;
	virtual ~TreeNode() = default;
};

class LoserTree {
private:
    int _run_num;
    std::vector<TreeNode*> tree;

public:
    LoserTree(int leaf_num);

    bool empty();

    TreeNode* top();

    void push(Item item, int run_index, int element_index);

    void adjust(int run_index);
};