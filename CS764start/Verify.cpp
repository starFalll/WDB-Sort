#include "Verify.h"

Bucket::Bucket(int bucket_capacity) : _bucket_index(0) {
    _bucket = new char[bucket_capacity];
}

Verify::Verify(){}

Verify::Verify(int row_size, unsigned long long file_size, unsigned long long memory_size) : 
    _row_size(row_size), _file_size(file_size), _mem_size(memory_size),
    _bucket_num(file_size / (memory_size * 0.5)) {

    _bucket_capacity = memory_size * 0.5 / _bucket_num;
    _hash_table = new Bucket*[_bucket_num];
    for(int i=0;i<_bucket_num;i++){
        _hash_table[i] = new Bucket(_bucket_capacity);
    }

    _input_file = new File(HDD_PATH_INPUT, __LONG_LONG_MAX__, HDD_BLOCK, std::ios::app);
	_output_file = new File(SSD_PATH_TEMP, __LONG_LONG_MAX__, HDD_BLOCK, std::ios::app);
}

Verify::~Verify(){
    for(int i=0;i<_bucket_num;i++){
        delete _hash_table[i];
    }
}


int Verify::hash(std::string record){
    // generate buckets id
    return std::stoi(record.substr(0, 7)) % _bucket_num;
}

void Verify::write_bucket(char* data, int length, int bucket_id, std::string dir_path){
    std::string file_name = std::to_string(bucket_id) + ".txt";
    // check if hash table dir exists
    int status = std::system(("[ -d '" + dir_path + "' ]").c_str());
    if(status != 0){
        status = std::system(("mkdir " + dir_path + " && touch " + file_name).c_str());
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

char* Verify::read_bucket(int bucket_id, std::string dir_path){
    // check if bucket exists
    std::string file_path = dir_path + std::to_string(bucket_id) + ".txt";
    // check if hash table dir exists
    int status = std::system(("[ -f '" + file_path + "' ]").c_str());
    if(status != 0){
        return nullptr;
    }

    // open file
    std::fstream bucket_file;
    bucket_file.open(file_path, std::ios::in | std::ios::app | std::ios::binary);

    bucket_file.seekg(0, std::ios::end);
    int bucket_size = bucket_file.tellg();
    char* bucket = new char[bucket_size];

    // write data
    bucket_file.seekg(0, std::ios::beg);
    bucket_file.read(bucket, bucket_size);
    // close file
    bucket_file.close();

    return bucket;
}

void Verify::create_hash_table(File* file, int batch_size, std::string dir_path, bool& order_status, bool check_order){
    // previous record (for check order)
    std::string prev;
    
    // scan input/output in batched, 100M per batch
    char* batch;
    int batch_num = ceil(file->getCurByte() / double(batch_size));
    int batch_id = 0;
    while(batch_id < batch_num){
        batch = file->read(batch_id * batch_size, batch_size);
        // traversal records in batch
        std::string batch_str(batch);
        for(int i=0;i<batch_str.size();i=i+_row_size){
            // get record
            std::string record = batch_str.substr(i, _row_size);
            // check order
            if(check_order){
                if(prev > record){
                    order_status = false;
                    // break;
                }
                prev = record;
            }
            // hash
            int bucket_id = hash(record);
            Bucket* bucket = _hash_table[bucket_id];
            // save in the bucket
            if(bucket->_bucket_index + _row_size > _bucket_capacity){
                // write file first;
                write_bucket(bucket->_bucket, bucket->_bucket_index, bucket_id, dir_path);
                // save in bucket
                strcpy(bucket->_bucket, record.c_str());
                // update bucket index
                bucket->_bucket_index = _row_size;
            }else{
                // save in bucket
                strcpy(&(bucket->_bucket[bucket->_bucket_index]), record.c_str());
                // update bucket index
                bucket->_bucket_index += _row_size;
            }
        }
        // if(check_order && !order_status){
        //     break;
        // }
        // writie bucket to the disk
        for(int i=0;i<_bucket_num;i++){
            if(_hash_table[i]->_bucket_index != 0){
                // write file
                write_bucket(_hash_table[i]->_bucket, _hash_table[i]->_bucket_index, i, dir_path);
            }
        }
        batch_id++;
    }
}

void Verify::reset_hashtable(){
     for(int i=0;i<_bucket_num;i++){
        _hash_table[i]->_bucket_index = 0;
        memset(_hash_table[i]->_bucket, '0', _bucket_capacity);
    }
}

void Verify::verify(){
    // check if the correct ascending order
    bool order_status = true;
    // check if the same set
    bool set_status = true;

    // scan input file
    create_hash_table(_input_file, 100*1024*1024, "./input_hash_table/", order_status);
    // reset
    reset_hashtable();
    // scan output file
    create_hash_table(_output_file, 100*1024*1024, "./output_hash_table/", order_status, true);

    // compare two hash tables
    for(int i=0;i<_bucket_num;i++){
        std::unordered_map<std::string, int> hash_map;
        // load two buckets
        char* bucket_1 = read_bucket(i, "./input_hash_table/");
        if(bucket_1 == nullptr){
            continue;
        }
        std::string bucket_1_str(bucket_1);
        for(int j=0;j<bucket_1_str.size();j=j+_row_size){
            std::string record = bucket_1_str.substr(j*_row_size, _row_size);
            hash_map[record]++;
        }
        // 
        char* bucket_2 = read_bucket(i, "./output_hash_table/");
        std::string bucket_2_str(bucket_2);
        for(int j=0;j<bucket_2_str.size();j=j+_row_size){
            std::string record = bucket_2_str.substr(j*_row_size, _row_size);
            if(hash_map.find(record) == hash_map.end()){
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

    std::cout<< "Verify Results:" << std::endl;
    std::cout<< "Sort Order: " << order_status << std::endl;
    std::cout<< "Sets of Rows & Values: " << set_status << std::endl;

}