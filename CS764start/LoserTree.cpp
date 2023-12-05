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

TreeNode::~TreeNode() {}

// override operator
bool TreeNode::operator < (const TreeNode & other) const {
	//return _value->fields[COMPARE_FIELD] < other._value->fields[COMPARE_FIELD];
    // int this_offset = _offset_value_code / 100;
    // int other_offset = other._offset_value_code / 100;

    // if (this_offset != other_offset) {
    //     return this_offset < other_offset;
    // } else {
    //     // if offset is same，extract offset_Data to compare
    //     int this_offset= _offset_value_code % 100;
    //     int other_offset = other._offset_value_code % 100;
    //     if (this_offset != other_offset) {
    //         return this_offset < other_offset;
    //     } else {
    //         int this_offsert_data = _offset_value_code % 10;
    //         int other_offsert_data = other._offset_value_code % 10;
    //         if (this_offsert_data != other_offsert_data) {
    //             return this_offsert_data < other_offsert_data;
    //         }
    //         // if OVC is same，then compare the whole string
    //         return *_value < *(other._value);
    //     }
    // }
    if(_offset_value_code != other._offset_value_code){
        return _offset_value_code < other._offset_value_code;
    }else{
        return strcmp(_value->fields[COMPARE_FIELD], other._value->fields[COMPARE_FIELD]) < 0;
    }
}

bool TreeNode::operator > (const TreeNode & other) const {
	//return _value->fields[COMPARE_FIELD] > other._value->fields[COMPARE_FIELD];

    // int this_offset = _offset_value_code / 100;
    // int other_offset = other._offset_value_code / 100;
    
    // // compare offset
    // if (this_offset != other_offset) {
    //     return this_offset > other_offset;
    // } else {
    //     // if offset is same，extract offset_Data to compare
    //     int this_offset = _offset_value_code % 100;
    //     int other_offset = other._offset_value_code % 100;
                
    //     if (this_offset != other_offset) {
    //         return this_offset > other_offset;
    //     } else {
    //         int this_offsert_data = _offset_value_code % 10;
    //         int other_offsert_data = other._offset_value_code % 10;
    //         if (this_offsert_data != other_offsert_data) {
    //             return this_offsert_data > other_offsert_data;
    //         }
    //         // if OVC is same，then compare the whole string
    //         return _value->fields[COMPARE_FIELD] > other._value->fields[COMPARE_FIELD];
    //     }
    // }
    if(_offset_value_code != other._offset_value_code){
        return _offset_value_code > other._offset_value_code;
    }else{
        return strcmp(_value->fields[COMPARE_FIELD], other._value->fields[COMPARE_FIELD]) > 0;
    }
}


// loser tree constructor
LoserTree::LoserTree (int32_t leaf_num, RowSize row_size):_leaf_num(leaf_num) {
    _tree = new TreeNode*[2 * leaf_num];
    for(uint32_t i=0;i<2 * leaf_num;i++){
        _tree[i] = new TreeNode();
    }
    _origin_leaf_num = leaf_num;
    ITEM_MIN = new Item(row_size, '0');
    ITEM_MAX = new Item(row_size, '9');
}

LoserTree::~LoserTree() {
    for(uint32_t i=0;i<2*_origin_leaf_num;i++){
        delete _tree[i];
    }
    delete [] _tree;

    delete ITEM_MIN;
    delete ITEM_MAX;
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
    // Get the string representation from Item
    std::string item_string = std::string(item->GetItemString());

    uint32_t offsetValueCode = 0; 
    // Every push needs to calculate the offset value code, based on the node that was just topped.
    // if (base_str_ptr)
    //     offsetValueCode = CalculateOffsetValueCode(base_str_ptr, item_ptr);
    offsetValueCode = CalculateOffsetValueCode(base_str_ptr, &item_string);

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
        if (_tree[node_index]->_offset_value_code == _tree[cmp_node_index]->_offset_value_code) {
            // _tree[node_index] is loser，update its ovc
            auto winner_str = std::string(_tree[node_index]->_value->GetItemString());
            auto loser_str = std::string(_tree[cmp_node_index]->_value->GetItemString());
            // Update the offset value code of the loser
            _tree[cmp_node_index]->_offset_value_code = CalculateOffsetValueCode(&winner_str, &loser_str);
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
    return ITEM_MIN;
}

Item* LoserTree::getMaxItem(){
    return ITEM_MAX;
}