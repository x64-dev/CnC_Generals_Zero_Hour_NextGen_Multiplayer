#ifndef __J2K__LZH__HuffStat_HPP__
#define __J2K__LZH__HuffStat_HPP__

#include "../CompLibHeader/HuffStatTmp.hpp"

class HuffStat {
public:
  HuffStat();
  virtual ~HuffStat();

protected:
  int makeSortedTmp( HuffStatTmpStruct* );

public:
  HUFFINT* stat;
};

#endif
