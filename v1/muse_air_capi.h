#include<stdint.h>



// 不使用BlindFast优化
typedef struct MuseAirNormal MuseAirNormal;

MuseAirNormal* NewMuseAirNormal();

void DeleteMuseAirNormal(MuseAirNormal* m);

/// @brief 计算64位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, type is uint64_t
void MuseAirNormal_LittleHash(MuseAirNormal *m, const void *data, uint64_t seed, void *result);

/// @brief 计算128位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, a pointer of uint8_t
void MuseAirNormal_LittleHash128(MuseAirNormal *m, const void *data, uint64_t seed, void *result);

/// @brief 计算64位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, type is uint64_t
void MuseAirNormal_BigHash(MuseAirNormal *m, const void *data, uint64_t seed, void *result);

/// @brief 计算128位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, a pointer of uint8_t
void MuseAirNormal_BigHash128(MuseAirNormal *m, const void *data, uint64_t seed, void *result);


// 使用BlindFast优化
typedef struct MuseAirOptimized MuseAirOptimized;

MuseAirOptimized* NewMuseAirOptimized();

void DeleteMuseAirOptimized(MuseAirOptimized* m);

/// @brief 计算64位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, type is uint64_t
void MuseAirOptimized_LittleHash(MuseAirOptimized *m, const void *data, uint64_t seed, void *result);

/// @brief 计算128位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, a pointer of uint8_t
void MuseAirOptimized_LittleHash128(MuseAirOptimized *m, const void *data, uint64_t seed, void *result);

/// @brief 计算64位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, type is uint64_t
void MuseAirOptimized_BigHash(MuseAirOptimized *m, const void *data, uint64_t seed, void *result);

/// @brief 计算128位哈希
/// @param m Instance which need to `New` one first.
/// @param data pointer of uint8_t
/// @param seed 
/// @param result result of hash calculation, a pointer of uint8_t
void MuseAirOptimized_BigHash128(MuseAirOptimized *m, const void *data, uint64_t seed, void *result);

// // 小端 little endian

// // 不使用BlindFast优化
// typedef struct MuseAirNormalLittle MuseAirNormalLittle;

// MuseAirNormalLittle* NewMuseAirNormalLittle();

// void MuseAirNormalLittle_Hash(MuseAirNormalLittle *m, const void *data, uint64_t seed, void *result);

// void MuseAirNormalLittle_Hash128(MuseAirNormalLittle *m, const void *data, uint64_t seed, void *result);


// // 使用BlindFast优化
// typedef struct MuseAirOptimizedLittle MuseAirOptimizedLittle;

// MuseAirOptimizedLittle* NewMuseAirOptimizedLittle();

// void MuseAirOptimizedLittle_Hash(MuseAirOptimizedLittle *m, const void *data, uint64_t seed, void *result);

// void MuseAirOptimizedLittle_Hash128(MuseAirOptimizedLittle *m, const void *data, uint64_t seed, void *result);



// // 大端 big endian

// // 不使用BlindFast优化
// typedef struct MuseAirNormalBig MuseAirNormalBig;

// MuseAirNormalBig* NewMuseAirNormalBig();

// void MuseAirNormalBig_Hash(MuseAirNormalBig* m, const void *data, uint64_t seed, void *result);

// void MuseAirNormalBig_Hash128(MuseAirNormalBig* m, const void *data, uint64_t seed, void *result);


// // 使用BlindFast优化
// typedef struct MuseAirOptimizedBig MuseAirOptimizedBig;

// MuseAirOptimizedBig* NewMuseAirOptimizedBig();

// void MuseAirOptimizedBig_Hash(MuseAirOptimizedBig *m, const void *data, uint64_t seed, void *result);

// void MuseAirOptimizedBig_Hash128(MuseAirOptimizedBig *m, const void *data, uint64_t seed, void *result);

