/*
 * Number types for FrostOS
 * - Ilonic 2026
 */

#ifndef NUMS_H_INCLUDE
#define NUMS_H_INCLUDE

typedef char      int8_t;
typedef short     int16_t;
typedef int       int32_t;
typedef long long int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef float     float32_t;
typedef double    float64_t;
typedef long double float96_t;

typedef unsigned int size_t;

#define INT8_MIN  0xFFFFFF80
#define INT8_MAX  0x7F
#define INT16_MIN 0xFFFF8000
#define INT16_MAX 0x7FFF
#define INT32_MIN 0x80000000 
#define INT32_MAX 0x7FFFFFFF 
#define INT64_MIN 0x7FFFFFFFFFFFFFFF 
#define INT64_MAX 0x800000000000000 

#endif
