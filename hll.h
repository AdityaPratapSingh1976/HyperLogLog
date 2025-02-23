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
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include<boost/serialization/set.hpp>
#include<boost/serialization/vector.hpp>
#include "xxHash/xxhash.h"
//Header files contain forward declaration of functions
//Header files contain definition and forward declared member functions for class

using BUCKET_TYPE = uint16_t;

template<typename T>
class hyperloglog{
    private :
    uint8_t b;  // initial bits for indexing
    size_t bucket_size;     // final bucket array size  := 2^b
    std::vector<BUCKET_TYPE>bucket;  // bucket array

    double _alpha;
    public :
    hyperloglog(uint8_t b); // bit constructor

    hyperloglog();  // default constructor set bucket size = 11

    uint64_t 
    hash64(const unsigned char* byte_array, size_t size);   // Hashing function

    void 
    add_element(const T& value);    // Adding Element 

    uint64_t
    get_cardinality();  // get approx cardinality of set
    
};


template<typename T>
inline 
hyperloglog<T>::hyperloglog(uint8_t _b): b{_b}{
    assert( _b > 0 && _b <= 29);
    bucket.resize(1 << _b);
    bucket_size = (1 << _b);
    switch ( (1 << _b))
    {
    case 16 : 
        _alpha = 0.673;
        break;
    case 32 :
        _alpha = 0.697;
        break;
    case 64 : 
        _alpha = 0.709;
        break;
    default:
        _alpha = 0.7213/(1 + 0.7213/(1 << _b));
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



template<typename T>
inline
void
hyperloglog<T>::add_element(const T &value){
    std::stringstream buffer;
    boost::archive::text_oarchive oa(buffer);

    oa << value;    // serialization of data into byte array
    std::string buff_str = buffer.str();
    unsigned char byte_arr[buff_str.length()];
    memcpy(byte_arr, buff_str.c_str(), buff_str.length());

    uint64_t hash_val = hash64(byte_arr, buff_str.length());    // generate 64bit hash

    std::bitset<64> bs(hash_val);

    int idx = 0;
    BUCKET_TYPE lzr = 1;
    BUCKET_TYPE p2 = 1;
    for(int i = 63; i >= 64 - b; --i){
        if(bs[i])idx += p2;
        p2 *= 2;
    }

    for(int i = 0; i < 64; ++i){
        if(bs[i] == 0)++lzr;
        else break;
    }
   
    bucket[idx] = std::max(lzr, bucket[idx]);
}


template<typename T>
inline
uint64_t 
hyperloglog<T>::get_cardinality(){
    long double pow_sum = 0;
    int v = 0;
    for(int i = 0; i < int(bucket_size); ++i){
        if(!bucket[i])++v;
        pow_sum += pow(2.0,- bucket[i]);
    }
    uint64_t raw_estimate = raw_estimate =_alpha * double(bucket_size * bucket_size)/ pow_sum;

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