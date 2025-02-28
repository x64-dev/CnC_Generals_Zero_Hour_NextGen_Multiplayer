#ifndef __J2K__LZH__LZHL_H__
#define __J2K__LZH__LZHL_H__

#include "../CompLibHeader/Basic.hpp"
#include "../CompLibHeader/Boolean.hpp"
#include <memory.h>
#include "../CompLibHeader/LZHMacro.hpp"


typedef struct { int _; }* LZHL_CHANDLE;
typedef struct { int _; }* LZHL_DHANDLE;
#define LZHL_CHANDLE_NULL ((LZHL_CHANDLE)0)
#define LZHL_DHANDLE_NULL ((LZHL_DHANDLE)0)

#ifdef __cplusplus
extern "C" {
#endif

LZHL_CHANDLE LZHLCreateCompressor( void );
size_t LZHLCompressorCalcMaxBuf( size_t );
size_t LZHLCompress( LZHL_CHANDLE, void* dst, const void* src, size_t srcSz );
void LZHLDestroyCompressor( LZHL_CHANDLE );

LZHL_DHANDLE LZHLCreateDecompressor( void );
int  LZHLDecompress( LZHL_DHANDLE, void* dst, size_t* dstSz, void* src, size_t* srcSz );
void LZHLDestroyDecompressor( LZHL_DHANDLE );

#ifdef __cplusplus
}
#endif

#endif
