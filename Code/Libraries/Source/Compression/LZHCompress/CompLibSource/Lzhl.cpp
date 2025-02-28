#ifndef __J2K__LZH__LZHL_CPP__
#define __J2K__LZH__LZHL_CPP__

#include "../CompLibHeader/lzhl.h"

#define LZHLINTERNAL

#include "../CompLibHeader/_huff.h"
#include "../CompLibHeader/_lz.h"

#ifdef __cplusplus
extern "C" {
#endif

LZHL_CHANDLE LZHLCreateCompressor( void ) 
{
  return (LZHL_CHANDLE)new LZHLCompressor();
}

size_t LZHLCompressorCalcMaxBuf( size_t sz ) 
{
  return LZHLCompressor::calcMaxBuf( sz );
}

size_t LZHLCompress( LZHL_CHANDLE compressor, void* dst, const void* src, size_t srcSz )
{
  return ((LZHLCompressor*)compressor)->compress( (BYTE*)dst, (const BYTE*)src, srcSz );
}

void LZHLDestroyCompressor( LZHL_CHANDLE compressor ) 
{
  delete (LZHLCompressor*)compressor;
}

//*****************************************************************************

LZHL_DHANDLE LZHLCreateDecompressor( void ) 
{
  return (LZHL_DHANDLE)new LZHLDecompressor();
}

int LZHLDecompress( LZHL_DHANDLE decompressor, void* dst, size_t* dstSz, void* src, size_t* srcSz )
{
  return ((LZHLDecompressor*)decompressor)->decompress( (BYTE*)dst, dstSz, (BYTE*)src, srcSz );
}

void LZHLDestroyDecompressor( LZHL_DHANDLE decompressor ) 
{
  delete (LZHLDecompressor*)decompressor;
}

#ifdef __cplusplus
}
#endif

#endif
