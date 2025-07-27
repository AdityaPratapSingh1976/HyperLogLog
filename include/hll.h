#ifndef MY_HLL_H
#define MY_HLL_H
#include<iostream>
#include<fstream>
#include<cmath>
#include<cassert>
#include<string>
#include<vector>
#include<cstring>
#include<bitset>
#include<mutex>
#include<boost/archive/text_oarchive.hpp>
#include<boost/archive/text_iarchive.hpp>
#include<boost/serialization/set.hpp>
#include<boost/serialization/vector.hpp>
#include "fasthash.h"
//Header files contain forward declaration of functions
//Header files contain definition and forward declared member functions for class

using BUCKET_TYPE = uint8_t;
using KEY_TYPE = uint64_t;

//extern uint64_t fasthash64(const unsigned char *data, size_t len);
void getbin(uint64_t num){
    for(int i = 63; i >= 0; --i){
        std::cout << bool(num & (1LL << i));
    }std::cout << std::endl;
}
template<typename T>
class hyperloglog{
    private :
    std::mutex                  mtx;                // Mutex for thread safety
    BUCKET_TYPE                 b;            // Initial bits for indexing
    int                         bucket_size;        // Final bucket array size  := 2^b
    std::vector<BUCKET_TYPE>    bucket;             // Bucket array
    KEY_TYPE                    (* hash64)(const unsigned char *byte_array, size_t size); // Hashing function pointer
    double                      CONSTANT = 0.79402; // Constant for cardinality estimation
    double                      alpha;              // Constant for cardinality estimation

    public :

    hyperloglog(BUCKET_TYPE b, KEY_TYPE (* hash64)(const unsigned char *byte_array, size_t size)); // bit constructor

    void 
    AddElem(const T& value);    // Adds element to the set

    int64_t
    get_cardinality();  // Returns approximate cardinality of the set
    
};



template<typename T>
inline 
hyperloglog<T>::hyperloglog(BUCKET_TYPE _b, KEY_TYPE (* hash64)(const unsigned char *byte_array, size_t size)){
    assert( _b >= 4 && _b <= 16);
    b = _b;
    bucket_size = (1 << _b); 
    bucket.resize(bucket_size + 10,0);
    this->hash64 = hash64;
    switch (bucket_size)
    {
        case 16 : 
            CONSTANT = 0.673;
            break;
        case 32 :
            CONSTANT = 0.697;
            break;
        case 64 : 
            CONSTANT = 0.709;
            break;
        default:
            CONSTANT = 0.7213/(1 + 0.7213/(bucket_size));
            break;
    }   
}

void binreverse(KEY_TYPE &key){
    uint64_t bit = 0;
    for(int i = 0; i < 64; ++i){
        bit = key & 1;
        key >>= 1;
        key |= (bit << 63);
    }
}

template<long unsigned int N>
void reverse(std::bitset<N> &b) {
    for(uint32_t i = 0; i < N/2; ++i) {
        bool t = b[i];
        assert(N >=  i + 1);
        b[i] = b[N-i-1];
        b[N-i-1] = t;
    }
}

template<typename T>
inline
void
hyperloglog<T>::AddElem(const T &value){
    printf("value of b is %d\n", this->b);
    std::stringstream buffer;
    boost::archive::text_oarchive oa(buffer);

    oa << value;    // serialization of data into byte array
    std::string buff_str = buffer.str();
    unsigned char* byte_arr = new unsigned char[buff_str.length()];
    memcpy(byte_arr, buff_str.c_str(), buff_str.length());

    KEY_TYPE hash_val = hash64(byte_arr, buff_str.length());
    std::bitset<64> bs(hash_val);
    int idx = 0;
    BUCKET_TYPE val = 1;
    for(int i = 63; i >= 64 - b; --i){
        idx += bs[i] * (1LL << (63 - i));
    }
    for(int i = 63 - b; i >= 0; --i){
        if(bs[i] == 0)++val;
        else break;
    }

    mtx.lock();
    assert(int(idx) < bucket_size && idx >=0);
    bucket[idx] = std::max(val, bucket[idx]);
    mtx.unlock();
    
    
    delete[] byte_arr;
}



template<typename T>
inline
int64_t 
hyperloglog<T>::get_cardinality(){
    long double pow_sum = 0;
    int v = 0;
    
    auto linearcount() = [&](int v)->double {
        return bucket_size * log(double(bucket_size)/v);
    };

    for(int i = 0; i < int(bucket_size); ++i){
        pow_sum += pow(2.0,- bucket[i]);
        if(bucket[i] == 0) ++v;
    }
    int64_t raw_estimate = CONSTANT * double(bucket_size * bucket_size)/ pow_sum;

    if(raw_estimate < 5/2 * bucket_size){
        if(v!= 0)
            raw_estimate = linearcount(v);
    }
    else if(raw_estimate > 1/30 * double(1LL<< 32)){
        raw_estimate = -1 * double(1LL << 32) * log(1 - double(raw_estimate)/(1LL << 32));
    }
    return int64_t(raw_estimate);
}

#endif