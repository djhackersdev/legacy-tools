
#ifndef __IIDXCOMMON_HPP
#define __IIDXCOMMON_HPP

#include "stdint.h"

namespace iidx
{

enum iidxVersion {
  FIRST = 0,
  SUBSTREAM = 1,
  SECOND = 2,
  THIRD = 3,
  FOURTH = 4,
  FIFTH = 5,
  SIXTH = 6,
  SEVENTH = 7,
  EIGTH = 8,
  NINETH = 9,
  TENTH = 10,
  RED = 11,
  HAPPPYSKY = 12,
  DISTORTED = 13,
  GOLD = 14,
  DJTROOPERS = 15,
  EMPRESS = 16,
  SIRIUS = 17,
  RESORTANTHEM = 18,
  LINCLE = 19
};


enum iidxLevel {
  NORMAL7 = 0,
  HYPER7 = 1,
  ANOTHER7 = 2,
  
  NORMAL14 = 3,
  HYPER14 = 4,
  ANOTHER14 = 5,
  BEGINNER = 6,

  EX = 7
};


enum saveoption
{
  OVERWRITE = 1<<1,
  NO_OVERWRITE = 1<<2,
  BACKUP = 1<<3,
  NO_BACKUP = 1<<4,

  DEFAULT = OVERWRITE | BACKUP
};

//exception classes
class filenotfound {};
class invalidfile {};
class songnotfound {};

}

#endif
