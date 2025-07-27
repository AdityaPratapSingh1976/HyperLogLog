#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "../include/fasthash.h"
#include "../include/hll.h"
#include "xxHash/xxhash.h"
#include <cmath>
using namespace std;
//uint64_t fasthash64(const unsigned char* data, size_t len);
uint64_t hash64(const unsigned char *byte_array, size_t size){
    return XXH64(byte_array, size,0);
}

int main(){
    long long int n = 10; // Number of elements to be added
    for(int sze = 4; sze <= 16; ++sze){
        cout <<"starting with b = " << sze << endl;
        hyperloglog<int> hll(sze, hash64);
        for(int i = 0; i < n; ++i){
            hll.AddElem(i);
        }
        cout << "Estimated cardinality: " << hll.get_cardinality() << endl; // Getting the estimated cardinality
        cout << "Error:" << ((hll.get_cardinality() - n)/double(n))*100<<'%' << endl; // Calculating the error
    }
    return 0;
}
