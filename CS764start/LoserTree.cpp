#include "LoserTree.h"
#include "ovc.h"
// constructor
TreeNode::TreeNode (Item* item, int32_t run_index, int32_t element_index){
	_value = item;
	_run_index = run_index;
	_element_index = element_index;
    _offset_value_code = 0;
}

TreeNode::TreeNode (){
	_value = &ITEM_MIN;
	_run_index = -1;
	_element_index = -1;
}

TreeNode::~TreeNode() {}

// override operator
bool TreeNode::operator < (const TreeNode & other) const {
	//return _value->fields[COMPARE_FIELD] < other._value->fields[COMPARE_FIELD];
    int this_offset = _offset_value_code / 100;
    int other_offset = other._offset_value_code / 100;

    if (this_offset != other_offset) {
        return this_offset > other_offset;
    } else {
        // if offset is same，extract offset_Data to compare
        int this_offset_Data = _offset_value_code % 100;
        int other_offset_Data = other._offset_value_code % 100;
        if (this_offset_Data != other_offset_Data) {
            return this_offset_Data < other_offset_Data;
        } else {
             // if OVC is same，then compare the whole string
            return _value->fields[COMPARE_FIELD] < other._value->fields[COMPARE_FIELD];
        }
    }
}

bool TreeNode::operator > (const TreeNode & other) const {
	//return _value->fields[COMPARE_FIELD] > other._value->fields[COMPARE_FIELD];

    int this_offset = _offset_value_code / 100;
    int other_offset = other._offset_value_code / 100;

    // compare offset
    if (this_offset != other_offset) {
        return this_offset < other_offset;
    } else {
        // if offset is same，extract offset_Data to compare
        int this_offset_Data = _offset_value_code % 100;
        int other_offset_Data = other._offset_value_code % 100;
                
        if (this_offset_Data != other_offset_Data) {
            return this_offset_Data > other_offset_Data;
        } else {
            // if OVC is same，then compare the whole string
            return _value->fields[COMPARE_FIELD] > other._value->fields[COMPARE_FIELD];
        }
    }
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
void LoserTree::push (const Item* item, int32_t run_index, int32_t element_index, const std::string& baseStr){
    // 从Item中获取字符串表示
    std::string itemStr = getItemString(item); 
    
    // 每一次push都需要计算偏移值代码，以刚刚top出去的节点为基准
    uint32_t offsetValueCode = calculate_offset_value_code(baseStr, itemStr);

    // update node value
    _tree[_leaf_num + run_index]->_value = item;
    _tree[_leaf_num + run_index]->_run_index = run_index;
    _tree[_leaf_num + run_index]->_element_index = element_index;
    _tree[_leaf_num + run_index]->_offset_value_code = offsetValueCode;
    // leaf to root update
    adjust(run_index, baseStr);
}

void LoserTree::adjust(int32_t run_index, const std::string& baseStr) {
    // to avoid overflow
    // calculate node index in the comparison
    uint32_t node_index = run_index + _leaf_num;
    uint32_t cmp_node_index = node_index / 2;

    // compare iteratively
    while(cmp_node_index > 0){
        if (*_tree[node_index] > *_tree[cmp_node_index]){
                if (_tree[node_index]->_offset_value_code == _tree[cmp_node_index]->_offset_value_code && baseStr != "") {
                // _tree[node_index] is loser，update its ovc
                std::string winner_str = getItemString(_tree[cmp_node_index]->_value);
                std::string loser_str = getItemString(_tree[node_index]->_value);
                // 更新失败者的偏移值代码
                _tree[node_index]->_offset_value_code = calculate_offset_value_code(winner_str, loser_str);
            }
            swap(_tree[node_index], _tree[cmp_node_index]);
        }
        cmp_node_index /= 2;
    }

    // replace the top element with the smallest one
    _tree[0] = _tree[node_index];

    // uint32_t node_index = run_index + _leaf_num;

    // while (node_index > 1) {
    //     uint32_t parent_index = node_index / 2;

    //     // 使用重构的比较函数
    //     if (*_tree[node_index] < *_tree[parent_index]) {
    //         // if OVC is same, then adjust offset value code
    //         if (_tree[node_index]->_offset_value_code == _tree[parent_index]->_offset_value_code) {
    //             // _tree[node_index] is loser，update its ovc
    //             std::string winner_str = getItemString(_tree[parent_index]->_value);
    //             std::string loser_str = getItemString(_tree[node_index]->_value);
    //             // 更新失败者的偏移值代码
    //             _tree[node_index]->_offset_value_code = calculate_offset_value_code(winner_str, loser_str);
    //         }

    //         // 交换节点
    //         std::swap(_tree[node_index], _tree[parent_index]);
    //     }

    //     // 移动到父节点，继续调整
    //     node_index = parent_index;
    // }

    // // 设置树顶节点
    // _tree[0] = _tree[node_index];
}

// update num_of_reset_nodes tree nodes to negative infinity
void LoserTree::reset(int32_t num_of_reset_nodes) {
    for(int32_t i=0;i<num_of_reset_nodes;i++){
        _tree[i]->_value = &ITEM_MIN;
        _tree[i]->_run_index = -1;
        _tree[i]->_element_index = -1;
    }
}