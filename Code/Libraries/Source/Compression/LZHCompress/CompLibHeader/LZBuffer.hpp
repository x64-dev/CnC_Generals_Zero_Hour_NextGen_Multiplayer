#ifndef __J2K__LZH__LZBuffer_HPP__
#define __J2K__LZH__LZBuffer_HPP__

class LZBuffer {
protected:
  inline LZBuffer();
  inline ~LZBuffer();

protected:
  inline static int _wrap( LZPOS pos );
  inline static int _distance( int diff );

  inline void _toBuf( BYTE );
  inline void _toBuf( const BYTE*, size_t sz );
  inline void _bufCpy( BYTE* dst, int pos, size_t sz );
  inline int _nMatch( int pos, const BYTE* p, int nLimit );

protected:
  BYTE* buf;
  LZPOS bufPos;
};

#endif
