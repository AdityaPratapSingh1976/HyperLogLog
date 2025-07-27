#include "../include/fasthash.h"

uint64_t fasthash64(const void *data, size_t len){
    uint64_t hash = 0;
    const uint8_t *data8 = static_cast<const uint8_t *>(data);
    for(size_t i = 0; i < len; ++i){
        hash = hash * 31 + data8[i];
    }
    return hash;
}