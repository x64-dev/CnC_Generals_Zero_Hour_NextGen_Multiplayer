#ifndef __J2K__LZH__LZ_CPP__
#define __J2K__LZH__LZ_CPP__

#define LZHLINTERNAL

#include "../CompLibHeader/_huff.h"
#include "../CompLibHeader/_lz.h"

inline LZHASH _calcHash( const BYTE* src ) 
{
  LZHASH hash = 0;
  const BYTE* pEnd = src + LZMATCH;
  for( const BYTE* p = src; p < pEnd ; ) 
  {
    UPDATE_HASH( hash, *p++ );
  }
  return hash;
}

#include "LZBuffer.cpp"
#include "LZHLCompressor.cpp"

#endif
