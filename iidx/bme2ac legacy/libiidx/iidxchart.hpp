#include "iidxcommon.hpp"
#include <list>

namespace iidx
{

class iidxChart_event
{
  uint32_t time;
  uint8_t type;
  uint8_t data1;
  uint16_t data2;
};

typedef std::list<iidxChart_event> eventlist;

class iidxChart
{
  //difficulty
public:
  uint8_t level;
  uint32_t notecount;
  //bool isdouble;
  
  eventlist events;

  //uint32 GetNotecount();
  iidxChart() { level = 0; notecount = 0; };
};

}
