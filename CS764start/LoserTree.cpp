#include "LoserTree.h"

TreeNode::TreeNode (Item item, uint32_t run_index, uint32_t element_index){
	_value = &item;
	_run_index = run_index;
	_element_index = element_index;
}

TreeNode::TreeNode (){
	_value = new Item(INT_MIN, INT_MIN, INT_MIN);
	_run_index = -1;
	_element_index = -1;
}

bool TreeNode::operator < (const TreeNode & other) const {
	return _value->fields[COMPARE_FIELD] < other._value->fields[COMPARE_FIELD];
}

bool TreeNode::operator > (const TreeNode & other) const {
	return _value->fields[COMPARE_FIELD] > other._value->fields[COMPARE_FIELD];
}

LoserTree::LoserTree (int run_num):_run_num(run_num) {
    tree.resize(2 * run_num+1);
    for(int i=0;i<tree.size();i++){
        tree[i] = new TreeNode();
    }
}

bool LoserTree::empty() {
    return this->top()->_element_index==-1 ? true : false;
}

TreeNode* LoserTree::top(){
    return tree[0];
}

void LoserTree::push (Item item, int run_index, int element_index){
    tree[_run_num + run_index] = new TreeNode(item, run_index, element_index);

    adjust(run_index);
}

void LoserTree::adjust(int run_index) {
    int node_index = run_index + _run_num;
    int cmp_node_index = node_index / 2;

    while(cmp_node_index > 0){
        if (tree[node_index] > tree[cmp_node_index]){
            swap(*tree[node_index], *tree[cmp_node_index]);
        }
        cmp_node_index /= 2;
    }

    tree[0] = tree[node_index];
}