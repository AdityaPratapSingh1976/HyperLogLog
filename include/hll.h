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

template<typename T>
class hyperloglog{
    private :
    std::mutex                  mtx;                // Mutex for thread safety
    BUCKET_TYPE                 b{11};              // Initial bits for indexing
    size_t                      bucket_size;        // Final bucket array size  := 2^b
    std::vector<BUCKET_TYPE>    bucket;             // Bucket array
    KEY_TYPE                    (* hash64)(const unsigned char *byte_array, size_t size); // Hashing function pointer
    double                      CONSTANT = 0.79402; // Constant for cardinality estimation
    double                      alpha;              // Constant for cardinality estimation

    public :

    hyperloglog(BUCKET_TYPE b, KEY_TYPE (* hash64)(const unsigned char *byte_array, size_t size)); // bit constructor

   // hyperloglog(BUCKET_TYPE b);
    void 
    AddElem(const T& value);    // Adds element to the set

    int64_t
    get_cardinality();  // Returns approximate cardinality of the set
    
};



template<typename T>
inline 
hyperloglog<T>::hyperloglog(BUCKET_TYPE _b, KEY_TYPE (* hash64)(const unsigned char *byte_array, size_t size)): b{_b}{
    assert( _b > 0 && _b <= 18);
    bucket_size = (1 << _b); 
    bucket.resize(bucket_size);
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

// template<typename T>
// inline 
// hyperloglog<T>::hyperloglog(BUCKET_TYPE _b, KEY_TYPE (* hash64)(const unsigned char *byte_array, size_t size)): hyperloglog<T>(_b){
//     this->hash64 = hash64;
// }

void binreverse(KEY_TYPE &key){
    uint64_t bit = 0;
    for(int i = 0; i < 64; ++i){
        bit = key & 1;
        key >>= 1;
        key |= (bit << 63);
    }
}

template<std::size_t N>
void reverse(std::bitset<N> &b) {
    for(std::size_t i = 0; i < N/2; ++i) {
        bool t = b[i];
        b[i] = b[N-i-1];
        b[N-i-1] = t;
    }
}

template<typename T>
inline
void
hyperloglog<T>::AddElem(const T &value){

    std::stringstream buffer;
    boost::archive::text_oarchive oa(buffer);

    oa << value;    // serialization of data into byte array
    std::string buff_str = buffer.str();
    unsigned char *byte_arr = new unsigned char[buff_str.length()];
    memcpy(byte_arr, buff_str.c_str(), buff_str.length());

    KEY_TYPE hash_val = hash64(byte_arr, buff_str.length());
    //-------------
    // binreverse(hash_val);
    // size_t idx = hash_val & ((1 << b) - 1);
    // hash_val >>= b; // Remove the bits used for indexing
    // BUCKET_TYPE lzr = __builtin_clzll(hash_val);

    // mtx.lock();
    // bucket[idx] = std::max(lzr, bucket[idx]);
    // mtx.unlock();
    //-------------

    //------------- 
    std::bitset<64> bs(hash_val);
    reverse(bs);
    int8_t p1 = 0;
    size_t idx = 0;
    
    /*
        compute index
    */
    BUCKET_TYPE lzr = 1;
    int8_t p2 = 1;
    for(int i = p1; i < p1 + b; ++i){
        if(bs[i])idx += p2;
        p2 *= 2;
    }
    /*
        compute value
    */
    for(int i = p1 + b; i < 64; ++i){
        if(bs[i] == 0)++lzr;
        else break;
    }

    BUCKET_TYPE val = lzr;
    mtx.lock();
    bucket[idx] = std::max(val, bucket[idx]);
    mtx.unlock();
    //-------------
    
    delete[] byte_arr;
}



template<typename T>
inline
int64_t 
hyperloglog<T>::get_cardinality(){
    long double pow_sum = 0;
    int v = 0;
    for(int i = 0; i < int(bucket_size); ++i){
        pow_sum += pow(2.0,- bucket[i]);
    }
    int64_t raw_estimate = CONSTANT * double(bucket_size * bucket_size)/ pow_sum;

    if(raw_estimate < 5/2 * bucket_size){
        if(v)
        raw_estimate = double(bucket_size) * log(double(bucket_size)/v);
        else 
        v = raw_estimate;
    }
    else if(raw_estimate > 1/30 * double(1LL<< 32)){
        raw_estimate = -1 * double(1LL << 32) * log(1 - double(raw_estimate)/(1LL << 32));
    }
    return int64_t(raw_estimate);
}

#endif