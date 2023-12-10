#include "Verify.h"
#include <cmath>
#include <unordered_map>

Bucket::Bucket(int bucket_capacity) : _bucket_index(0) {
    _bucket = new char[bucket_capacity];
    memset(_bucket, 0, bucket_capacity);
}

Bucket::~Bucket() {
    delete [] _bucket;
}

Verify::Verify() {}

Verify::Verify(int row_size, unsigned long long file_size, unsigned long long memory_size) : 
    _row_size(row_size), _file_size(file_size), _bucket_num(ceil(file_size / (memory_size * 0.5))) {

    _batch_size = (memory_size / 2)/_row_size * _row_size;
    _bucket_capacity = _batch_size / _bucket_num;
    _hash_table = new Bucket*[_bucket_num];
    for(int i=0;i<_bucket_num;i++){
        _hash_table[i] = new Bucket(_bucket_capacity);
    }
    _input_bucket_size = new int[_bucket_num];
    memset(_input_bucket_size, 0, sizeof(int) * _bucket_num);
    _output_bucket_size = new int[_bucket_num];
    memset(_output_bucket_size, 0, sizeof(int) * _bucket_num);

    _input_file = new File(HDD_PATH_INPUT, __LONG_LONG_MAX__, HDD_BLOCK, std::ios::app, HDD, _row_size);
	_output_file = new File(RES_HDD_PATH, __LONG_LONG_MAX__, HDD_BLOCK, std::ios::app, HDD, _row_size);

    int status = std::system(("[ -d './temp/' ]"));
    if(status == 0){
        status = std::system(("rm -rf ./temp/" ));
    }
    status = std::system(("[ -d './input_hash_table/' ]"));
    if(status == 0){
        status = std::system(("rm -rf ./input_hash_table/" ));
    }
    status = std::system(("[ -d './output_hash_table/' ]"));
    if(status == 0){
        status = std::system(("rm -rf ./output_hash_table/"));
    }
}

Verify::~Verify(){
    for(int i=0;i<_bucket_num;i++){
        delete _hash_table[i];
    }
    delete [] _hash_table;

    delete [] _input_bucket_size;
    delete [] _output_bucket_size;
    delete _input_file;
    delete _output_file;
}


int Verify::hash(std::string record){
    try {
        int value = std::stoi(record.substr(0, 7));
        return value % _bucket_num;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
    }
    return -1;
}

void Verify::write_bucket(char* data, int length, int bucket_id, std::string dir_path){
    std::string file_name = std::to_string(bucket_id) + ".txt";
    // check if hash table dir exists
    int status = std::system(("[ -d '" + dir_path + "' ]").c_str());
    if(status != 0){
        status = std::system(("mkdir " + dir_path + " && touch " + dir_path + file_name).c_str());
    }else{
        // check if bucket file exists
        status = std::system(("[ -f '" + dir_path + file_name + "' ]").c_str());
        if(status != 0){
            status = std::system(("touch " + dir_path + file_name).c_str());
        }
    }

    // open file
    std::fstream bucket_file;
    bucket_file.open(dir_path+file_name, std::ios::out | std::ios::in | std::ios::app | std::ios::binary);
    // write data
    bucket_file.write(data, length);
    // close file
    bucket_file.close();
}

std::string Verify::read_bucket(int bucket_id, std::string dir_path, bool is_output){
    // check if bucket exists
    std::string file_path = dir_path + std::to_string(bucket_id) + ".txt";
    // check if hash table dir exists
    int status = std::system(("[ -f '" + file_path + "' ]").c_str());
    if(status != 0){
        return "";
    }

    int bucket_size = is_output ? _output_bucket_size[bucket_id] : _input_bucket_size[bucket_id];
    char* bucket = new char[bucket_size];
    memset(bucket, 0, sizeof(bucket_size));

    // open file
    std::fstream bucket_file;
    bucket_file.open(file_path, std::ios::in | std::ios::binary);
    // write data
    bucket_file.seekg(0, std::ios::beg);
    bucket_file.read(bucket, bucket_size);
    // close file
    bucket_file.close();

    auto str = std::string(bucket, bucket_size);
    delete [] bucket;
    return str;
}

void Verify::create_hash_table(File* file, std::string dir_path, bool& order_status, bool is_output){
    // previous record key(for check order)
    std::string prev;
    std::string record;
    std::string batch_str;
    
    // scan input/output in batched, 100M per batch
    char* batch;
    int batch_num = ceil(_file_size / double(_batch_size));
    int batch_id = 0;
    while(batch_id < batch_num){
        int32_t read_size = 0;
        batch = file->read((uint64_t)batch_id * (uint64_t)_batch_size, _batch_size, &read_size);
        // traversal records in batch
        batch_str = std::string(batch, read_size);
        delete [] batch;
        for(int i=0;i<batch_str.size();i=i+_row_size){
            // get record
            record = batch_str.substr(i, _row_size);
            // hash
            int bucket_id = hash(record);
            if (bucket_id < 0) continue;
            // check order
            if(is_output){
                if(prev > record.substr(0, _row_size / 3)){
                    order_status = false;
                    printf("order is not correct, prev:%s, cur:%s\n", prev.c_str(), record.c_str());
                    break;
                }
                prev = record.substr(0, _row_size / 3);
            }
            
            Bucket* bucket = _hash_table[bucket_id];
            if(is_output){
                _output_bucket_size[bucket_id] += _row_size;
            }else{
                _input_bucket_size[bucket_id] += _row_size;
            }
            // save in the bucket
            if(bucket->_bucket_index + _row_size > _bucket_capacity){
                // write file first;
                write_bucket(bucket->_bucket, bucket->_bucket_index, bucket_id, dir_path);
                // save in bucket
                memcpy(bucket->_bucket, record.c_str(), _row_size);
                // update bucket index
                bucket->_bucket_index = _row_size;
            }else{
                // save in bucket
                memcpy(&(bucket->_bucket[bucket->_bucket_index]), record.c_str(), _row_size);
                // update bucket index
                bucket->_bucket_index += _row_size;
            }
        }
        // writie bucket to the disk
        for(int i=0;i<_bucket_num;i++){
            if(_hash_table[i]->_bucket_index != 0){
                // write file
                write_bucket(_hash_table[i]->_bucket, _hash_table[i]->_bucket_index, i, dir_path);
                _hash_table[i]->_bucket_index = 0;
            }
        }
        batch_id++;
    }
}

void Verify::verify(){
    // check if the correct ascending order
    bool order_status = true;
    // check if the same set
    bool set_status = true;

    // scan input file
    create_hash_table(_input_file, "./input_hash_table/", order_status, false);
    // scan output file
    create_hash_table(_output_file, "./output_hash_table/", order_status, true);

    // current record
    std::string record;
    // bucket in input
    std::string bucket_1_str;
    // bucket in output
    std::string bucket_2_str;
    // hash map
    std::unordered_map<std::string, int> hash_map;

    // compare two hash tables
    for(int i=0;i<_bucket_num;i++){
        if(_input_bucket_size[i] != _output_bucket_size[i]){
            std::cout<< "verifying failed, bucket:"<<i<<" input size:" << _input_bucket_size[i] <<" output_size:"<<_output_bucket_size[i]<<std::endl;
            set_status = false;
            break;
        }
        
        // clear hash map
        hash_map.clear();
        bucket_1_str.clear();
        bucket_2_str.clear();

        // load two buckets   
        bucket_1_str = read_bucket(i, "./input_hash_table/", false);
        if(bucket_1_str == ""){
            continue;
        }
        for(int j=0;j<bucket_1_str.size();j=j+_row_size){
            record = bucket_1_str.substr(j, _row_size);
            hash_map[record]++;
        }
        // compare
        bucket_2_str = read_bucket(i, "./output_hash_table/", true);
        if(bucket_2_str == ""){
            continue;
        }
        // delete [] bucket_2;
        for(int j=0;j<bucket_2_str.size();j=j+_row_size){
            record = bucket_2_str.substr(j, _row_size);
            if(hash_map.find(record) == hash_map.end()){
                std::cout<< "verifying failed, record: " << record <<" not int input bucket " << i <<std::endl;
                set_status = false;
                break;
            }else{
                hash_map[record]--;
                if(hash_map[record] == 0){
                    hash_map.erase(record);
                }
            }
        }
        if(!set_status){
            break;
        }
        if(hash_map.size() != 0){
            set_status = false;
            break;
        }

    }

    std::string order_res = order_status ? "True" : "False";
    std::string set_res = set_status ? "True" : "False";

	printf (
	"||---------------------[Hash Table Status]----------------------||\n"
	"||%-25s|   %33d||\n"
    "||%-25s|   %33d||\n"
	"||------------------------Verify Results------------------------||\n"
	"||%-50s| %10s||\n"
	"||%-50s| %10s||\n",
	"Number of input buckets", _bucket_num,
    "Number of output buckets", _bucket_num,
    "Is output file ordered?", order_res.c_str(),
    "Are records in input match with output?",set_res.c_str());
    

}
