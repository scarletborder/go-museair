#include <cstring> // for memset
#include "MuseAir.hpp"

extern "C"{
    #include "muse_air_capi.h"
}

struct MuseAirNormal: MuseAir<false>{
    MuseAirNormal(): MuseAir<false>() {}
    ~MuseAirNormal() {}
};

struct MuseAirOptimized: MuseAir<true>{
    MuseAirOptimized(): MuseAir<true>() {}
    ~MuseAirOptimized() {}
};

MuseAirNormal* NewMuseAirNormal(){
    auto m = new MuseAirNormal();
    return m;
}

void DeleteMuseAirNormal(MuseAirNormal* m){
    delete m;
}

void MuseAirNormal_LittleHash(MuseAirNormal *m, const void *data, uint64_t seed, void *result){
    m->hash<false>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}

void MuseAirNormal_LittleHash128(MuseAirNormal *m, const void *data, uint64_t seed, void *result){
    m->hash_128<false>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}


void MuseAirNormal_BigHash(MuseAirNormal *m, const void *data, uint64_t seed, void *result){
    m->hash<true>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}

void MuseAirNormal_BigHash128(MuseAirNormal *m, const void *data, uint64_t seed, void *result){
    m->hash_128<true>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}



MuseAirOptimized* NewMuseAirOptimized(){
    auto m = new MuseAirOptimized();
    return m;
}

void DeleteMuseAirOptimized(MuseAirOptimized* m){
    delete m;
}

void MuseAirOptimized_LittleHash(MuseAirOptimized *m, const void *data, uint64_t seed, void *result){
    m->hash<false>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}

void MuseAirOptimized_LittleHash128(MuseAirOptimized *m, const void *data, uint64_t seed, void *result){
    m->hash_128<false>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}


void MuseAirOptimized_BigHash(MuseAirOptimized *m, const void *data, uint64_t seed, void *result){
    m->hash<true>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}

void MuseAirOptimized_BigHash128(MuseAirOptimized *m, const void *data, uint64_t seed, void *result){
    m->hash_128<true>( data, strlen( reinterpret_cast<const char*>( data ) ), seed, result);
}