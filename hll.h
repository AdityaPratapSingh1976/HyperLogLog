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
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include<boost/serialization/set.hpp>
#include<boost/serialization/vector.hpp>
#include "xxHash/xxhash.h"
//Header files contain forward declaration of functions
//Header files contain definition and forward declared member functions for class

using BUCKET_TYPE = int8_t;

template<typename T>
class hyperloglog{
    private :
    std::mutex mtx;
    BUCKET_TYPE b;  // initial bits for indexing
    size_t bucket_size;     // final bucket array size  := 2^b
    std::vector<BUCKET_TYPE>bucket;  // bucket array

    double CONSTANT = 0.79402;
    public :
    hyperloglog(uint8_t b); // bit constructor

    hyperloglog();  // default constructor set bucket size = 11

    uint64_t 
    hash64(const unsigned char* byte_array, size_t size);   // Hashing function

    void 
    AddElem(const T& value);    // Adding Element 

    uint64_t
    get_cardinality();  // get approx cardinality of set
    
};



template<typename T>
inline 
hyperloglog<T>::hyperloglog(uint8_t _b): b{_b}{
    assert( _b > 0 && _b <= 29);
    bucket_size = (1 << _b); 
    bucket.resize(bucket_size);
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



template<typename T >
inline 
uint64_t 
hyperloglog<T>::hash64 (const unsigned char* byte_array, size_t size){
    /*
        Implementation of good hashing function
        which convert byte array to 64bit hash-key value
    */
    return XXH64(byte_array,size,0);
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
    unsigned char byte_arr[buff_str.length()];
    memcpy(byte_arr, buff_str.c_str(), buff_str.length());

    uint64_t hash_val = hash64(byte_arr, buff_str.length());    // generate 64bit hash

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
}



template<typename T>
inline
uint64_t 
hyperloglog<T>::get_cardinality(){
    long double pow_sum = 0;
    int v = 0;
    for(int i = 0; i < int(bucket_size); ++i){
        pow_sum += pow(2.0,- bucket[i]);
    }
    uint64_t raw_estimate = CONSTANT * double(bucket_size * bucket_size)/ pow_sum;

    if(raw_estimate < 5/2 * bucket_size){
        if(v)
        raw_estimate = double(bucket_size) * log(double(bucket_size)/v);
        else 
        v = raw_estimate;
    }
    else if(raw_estimate > 1/30 * double(1LL<< 32)){
        raw_estimate = -1 * double(1LL << 32) * log(1 - double(raw_estimate)/(1LL << 32));
    }
    return raw_estimate;
}

#endif
