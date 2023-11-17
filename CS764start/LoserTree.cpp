#include "LoserTree.h"

// constructor
TreeNode::TreeNode (Item* item, int32_t run_index, int32_t element_index){
	_value = item;
	_run_index = run_index;
	_element_index = element_index;
}

TreeNode::TreeNode (){
	_value = &ITEM_MIN;
	_run_index = -1;
	_element_index = -1;
}

TreeNode::~TreeNode() {}

// override operator
bool TreeNode::operator < (const TreeNode & other) const {
	return _value->fields[COMPARE_FIELD] < other._value->fields[COMPARE_FIELD];
}

bool TreeNode::operator > (const TreeNode & other) const {
	return _value->fields[COMPARE_FIELD] > other._value->fields[COMPARE_FIELD];
}


// loser tree constructor
LoserTree::LoserTree (int32_t leaf_num):_leaf_num(leaf_num) {
    _tree = new TreeNode*[2 * leaf_num];
    for(uint32_t i=0;i<2 * leaf_num;i++){
        _tree[i] = new TreeNode();
    }
}

LoserTree::~LoserTree() {
    for(uint32_t i=0;i<2*_leaf_num;i++){
        delete _tree[i];
    }
    delete [] _tree;
}

// check if loser tree is empty (no valid node)
bool LoserTree::empty() {
    return this->top()->_element_index==-1 ? true : false;
}

// return the smallest node element in the tree 
TreeNode* LoserTree::top(){
    return _tree[0];
}

// push new value into the tree
void LoserTree::push (const Item* item, int32_t run_index, int32_t element_index){
    // update node value
    _tree[_leaf_num + run_index]->_value = item;
    _tree[_leaf_num + run_index]->_run_index = run_index;
    _tree[_leaf_num + run_index]->_element_index = element_index;

    // leaf to root update
    adjust(run_index);
}

void LoserTree::adjust(int32_t run_index) {
    // to avoid overflow
    // calculate node index in the comparison
    uint32_t node_index = run_index + _leaf_num;
    uint32_t cmp_node_index = node_index / 2;

    // compare iteratively
    while(cmp_node_index > 0){
        if (*_tree[node_index] > *_tree[cmp_node_index]){
            swap(_tree[node_index], _tree[cmp_node_index]);
        }
        cmp_node_index /= 2;
    }

    // replace the top element with the smallest one
    swap(_tree[0], _tree[node_index]);
}

// update num_of_reset_nodes tree nodes to negative infinity
void LoserTree::reset(int32_t num_of_reset_nodes, Item* value) {
    for(int32_t i=0;i<num_of_reset_nodes;i++){
        _tree[i]->_value = value;
        _tree[i]->_run_index = -1;
        _tree[i]->_element_index = -1;
    }
}