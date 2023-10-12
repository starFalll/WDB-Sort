#include "LoserTree.h"

TreeNode::TreeNode (Item item, uint32_t run_index, uint32_t element_index){
	_value = &item;
	_run_index = run_index;
	_element_index = element_index;
}

TreeNode::TreeNode (){
	_value = new Item(0, 0, 0);
	_run_index = -1;
	_element_index = -1;
}

TreeNode::~TreeNode() {}

bool TreeNode::operator < (const TreeNode & other) const {
	return _value->fields[COMPARE_FIELD] < other._value->fields[COMPARE_FIELD];
}

bool TreeNode::operator > (const TreeNode & other) const {
	return _value->fields[COMPARE_FIELD] > other._value->fields[COMPARE_FIELD];
}

LoserTree::LoserTree (uint32_t run_num):_run_num(run_num) {
    tree.resize(2 * run_num+1);
    for(uint32_t i=0;i<tree.size();i++){
        tree[i] = new TreeNode();
    }
}

LoserTree::~LoserTree() {}

bool LoserTree::empty() {
    return this->top()->_element_index==-1 ? true : false;
}

TreeNode* LoserTree::top(){
    return tree[0];
}

void LoserTree::push (Item item, uint32_t run_index, uint32_t element_index){
    tree[_run_num + run_index]->_value = &item;
    tree[_run_num + run_index]->_run_index = run_index;
    tree[_run_num + run_index]->_element_index = element_index;

    adjust(run_index);
}

void LoserTree::adjust(uint32_t run_index) {
    uint32_t node_index = run_index + _run_num;
    uint32_t cmp_node_index = node_index / 2;

    while(cmp_node_index > 0){
        if (tree[node_index] > tree[cmp_node_index]){
            swap(*tree[node_index], *tree[cmp_node_index]);
        }
        cmp_node_index /= 2;
    }

    tree[0] = tree[node_index];
}