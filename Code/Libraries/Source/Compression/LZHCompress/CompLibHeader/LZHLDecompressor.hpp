#ifndef __J2K__LZH__LZHLDecompressor_HPP__
#define __J2K__LZH__LZHLDecompressor_HPP__

#include "Boolean.hpp"

class LZHLDecompressor : private LZBuffer, private LZHLDecoderStat {
private:
  UINT32 bits;
  int nBits;

public:
  LZHLDecompressor();
  virtual ~LZHLDecompressor();
  BOOL decompress( BYTE* dst, size_t* dstSz, const BYTE* src, size_t* srcSz );

private:
  inline int _get( const BYTE*& src, const BYTE* srcEnd, int n );
};

#endif
