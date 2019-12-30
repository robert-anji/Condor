// $RCSfile:$
// $Revision:$

#ifndef SIMULATORTIME_CLASS
#define SIMULATORTIME_CLASS

#include "SimulatorDefines.h"

class SimulatorTime
{
public:
  SimulatorTime();
  SimulatorTime(eTimeScale tms);
 ~SimulatorTime();

  eTimeScale  TimeScaleGet() const { return mTimeScale; }
  eTimeScale  TimeScaleSet(eTimeScale tm_scale);
  unsigned    TimeGet     () const { return mTime; }
  unsigned    TimeGetUnsigned() const { return mTime; }
  void        TimeSet     (SimulatorTime& tms);
  const char* TimeGetStr  ();
  //
  unsigned   Convert(eTimeScale tms) const;

  SimulatorTime& operator=(unsigned u);
  SimulatorTime& operator=(const SimulatorTime& tms);
  //
  SimulatorTime& operator+ (const SimulatorTime& tms);
  SimulatorTime& operator+=(const SimulatorTime& tms);
  SimulatorTime& operator+ (unsigned u);
  SimulatorTime& operator+=(unsigned u);
  SimulatorTime& operator- (const SimulatorTime& tms);
  SimulatorTime& operator- (unsigned u);
  //
  bool           operator> (const SimulatorTime& tms) const;
  bool           operator>=(const SimulatorTime& tms) const;
  bool           operator< (const SimulatorTime& tms) const;
  bool           operator<=(const SimulatorTime& tms) const;
  bool           operator==(const SimulatorTime& tms) const;

  bool           operator==(unsigned tm) const;
  bool           operator!=(unsigned tm) const;

private:
  unsigned     mTime;
  unsigned     mTimeMsb;
  eTimeScale   mTimeScale;
  char         mTimeStr[65];
};

#endif
