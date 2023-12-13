#include "LoserTree.h"

// constructor
TreeNode::TreeNode (Item* item, int32_t run_index, int32_t element_index){
	_value = item;
	_run_index = run_index;
	_element_index = element_index;
    _offset_value_code = 0;
}

TreeNode::TreeNode (){
	_value = nullptr;
	_run_index = -1;
	_element_index = -1;
}

TreeNode::~TreeNode() {
    // if (_value) {
    //     delete _value;
    //     _value = nullptr;
    // }
}

// override operator
bool TreeNode::operator < (const TreeNode & other) const {
    if(_offset_value_code != 0 && other._offset_value_code != 0 && 
         _offset_value_code != other._offset_value_code){
        return _offset_value_code < other._offset_value_code;
    }else{
        // full value compare
        return *_value < *other._value;
    }
}

bool TreeNode::operator > (const TreeNode & other) const {
    if(_offset_value_code != 0 && other._offset_value_code != 0 && 
        _offset_value_code != other._offset_value_code){
        return _offset_value_code > other._offset_value_code;
    }else{
        // full value compare
        return *_value > *other._value;
    }
}


// loser tree constructor
LoserTree::LoserTree (int32_t leaf_num, RowSize row_size):_leaf_num(leaf_num) {
    _tree = new TreeNode*[2 * leaf_num];
    for(uint32_t i=0;i<2 * leaf_num;i++){
        _tree[i] = new TreeNode();
    }
    _origin_leaf_num = leaf_num;
    _row_size = row_size;
}

LoserTree::~LoserTree() {
    for(uint32_t i=0;i<2*_origin_leaf_num;i++){
        if (_tree[i]->_element_index == -1) {
            delete _tree[i]->_value;
        }
        delete _tree[i];
        _tree[i] = nullptr;
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
void LoserTree::push(Item* item, int32_t run_index, int32_t element_index, std::string* base_str_ptr){

    uint32_t offsetValueCode = 0; 
    // Every push needs to calculate the offset value code, based on the node that was just topped.
    if (base_str_ptr) {
        offsetValueCode = CalculateOffsetValueCode((*base_str_ptr).c_str(), item->GetItemString());
    }
        

    // update node value
    _tree[_leaf_num + run_index]->_value = item;
    _tree[_leaf_num + run_index]->_run_index = run_index;
    _tree[_leaf_num + run_index]->_element_index = element_index;
    _tree[_leaf_num + run_index]->_offset_value_code = offsetValueCode;
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
        if (_tree[node_index]->_offset_value_code == 0 || _tree[cmp_node_index]->_offset_value_code == 0 || 
            _tree[node_index]->_offset_value_code == _tree[cmp_node_index]->_offset_value_code) {
            // _tree[node_index] is loser，update its ovc
            char* winner = _tree[node_index]->_value->GetItemString();
            char* loser = _tree[cmp_node_index]->_value->GetItemString();
            // Update the offset value code of the loser
            _tree[cmp_node_index]->_offset_value_code = CalculateOffsetValueCode(winner, loser);
        }
        cmp_node_index /= 2;
    }

    // replace the top element with the smallest one
    swap(_tree[0], _tree[node_index]);
}

// update num_of_reset_nodes tree nodes to negative infinity
void LoserTree::reset(int32_t num_of_reset_nodes, Item* value) {
    _leaf_num = num_of_reset_nodes;
    for(int32_t i=0;i<num_of_reset_nodes;i++){
        if (_tree[i]->_value) {
            delete _tree[i]->_value;
            _tree[i]->_value = nullptr;
        }
        _tree[i]->_value = value;
        _tree[i]->_run_index = -1;
        _tree[i]->_element_index = -1;
        _tree[i]->_offset_value_code = 0;
    }
}

std::string LoserTree::getvalue(int i){
    return _tree[i]->_value->fields[0];
}

Item* LoserTree::getMinItem(){
    return new Item(_row_size, '0');
}

Item* LoserTree::getMaxItem(){
    return new Item(_row_size, '9');
}