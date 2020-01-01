// $RCSfile:$
// $Revision:$

#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "SimulatorTime.h"

#include "Global.h"

M_ExternConstructDestructCountInt;

// Rename to SimulationTime later.
SimulatorTime::SimulatorTime() :
  mTime         (0),
  mTimeMsb      (0),
  mTimeScale    (eTimeScale_Invalid)
{
  M_UpdConstructCount(eType_SimulatorTime);
}

SimulatorTime::SimulatorTime(eTimeScale tm_scale) :
  mTime         (0),
  mTimeMsb      (0),
  mTimeScale    (tm_scale)
{
  mTimeStr[0] = '\0';

  M_UpdConstructCount(eType_SimulatorTime);
}

SimulatorTime::~SimulatorTime()
{
  M_UpdDestructCount(eType_SimulatorTime);
}

eTimeScale SimulatorTime::TimeScaleSet(eTimeScale tm_scale)
{
  // Allowed to be set only once.
  //assert(mTimeScale == eTimeScale_Invalid);

  mTimeScale = tm_scale;
}

const char* SimulatorTime::TimeGetStr()
{
  sprintf(mTimeStr, "%u", mTime);

  return mTimeStr;
}

void SimulatorTime::TimeSet(SimulatorTime& tms)
{
  tms.mTime      = mTime;
  tms.mTimeScale = mTimeScale;
}

unsigned SimulatorTime::Convert(eTimeScale tms) const
{
  // Only allow conversion to a higher resolution time scale.
//Assert(tms>=mTimeScale, "Only allowed to convert time to a higher resolution");
  assert(tms>=mTimeScale);

  if (tms == eTimeScale_Default) return mTime;

  // Check for overflow.
  if (mTime > UINT_MAX/(1000^(tms-mTimeScale))) {
    assert(0);
  //AssertMsg("Exceeded maximum time unit: UINT_MAX");
  }
 
  // FIX: ^ is not a power-of operator, need to write own routine.
  return (mTime * 1000^(tms-mTimeScale));
}

SimulatorTime& SimulatorTime::operator=(unsigned u)
{
  mTime = u;

  return *this;
}

SimulatorTime& SimulatorTime::operator=(const SimulatorTime& tms)
{
  mTime      = tms.mTime;
  mTimeScale = tms.mTimeScale;

  return *this;
}

SimulatorTime& SimulatorTime::operator+(const SimulatorTime& tms)
{
  if (mTimeScale == tms.mTimeScale) { mTime += tms.mTime; }
  else                              { mTime += Convert(mTimeScale); }

  return *this;
}

SimulatorTime& SimulatorTime::operator+=(const SimulatorTime& tms)
{
  if (mTimeScale == tms.mTimeScale) { mTime += tms.mTime; }
  else                              { mTime += Convert(mTimeScale); }

  return *this;
}

SimulatorTime& SimulatorTime::operator+(unsigned u)
{
  mTime += u;

  return *this;
}

SimulatorTime& SimulatorTime::operator+=(unsigned u)
{
  mTime += u;

  return *this;
}

SimulatorTime& SimulatorTime::operator-(const SimulatorTime& tms)
{
  unsigned tm;

  if (mTimeScale == tms.mTimeScale) {
    // For now we support only 32 bit time for subtraction.
    assert(mTime >= tms.mTime);

    mTime -= tms.mTime;
  }
  else if (mTimeScale > tms.mTimeScale) {
    tm = tms.Convert(mTimeScale);

    // For now we support only 32 bit time for subtraction.
    assert (mTime >= tm);

    mTime -= tm;
  }
  else {
    assert(0); // Cannot subtract a low resolution from a high resolution,
               // example: 1ms - 20ns; // would need time scale change on lhs.
  }

  return *this;
}

SimulatorTime& SimulatorTime::operator-(unsigned u)
{
  // For now we support only 32 bit time for subtraction.
  assert(mTime >= u);

  mTime -= u;
}

bool SimulatorTime::operator>(const SimulatorTime& tms) const
{
  if (mTimeScale==tms.mTimeScale) {
    return (mTime > tms.mTime);
  }

  unsigned tm;

  if (mTimeScale < tms.mTimeScale)
  {
    tm = Convert(tms.mTimeScale);
    
    return (tm > tms.mTime);
  }

  tm = tms.Convert(mTimeScale);
    
  return (mTime > tm);
}

bool SimulatorTime::operator>=(const SimulatorTime& tms) const
{
  if (*this >  tms) return TRUE;
  if (*this == tms) return TRUE;

  return FALSE;
}

bool SimulatorTime::operator<(const SimulatorTime& tms) const
{
  if (mTimeScale==tms.mTimeScale) {
    return (mTime < tms.mTime);
  }

  unsigned tm;

  if (mTimeScale < tms.mTimeScale)
  {
    tm = Convert(tms.mTimeScale);
    
    return (tm < tms.mTime);
  }

  tm = tms.Convert(mTimeScale);
    
  return (mTime < tm);
}

bool SimulatorTime::operator<=(const SimulatorTime& tms) const
{
  if (*this <  tms) return TRUE;
  if (*this == tms) return TRUE;

  return FALSE;
}

bool SimulatorTime::operator==(const SimulatorTime& tms) const
{
  if (mTimeScale==tms.mTimeScale) {
    return (mTime == tms.mTime);
  }

  unsigned tm;

  if (mTimeScale < tms.mTimeScale)
  {
    tm = Convert(tms.mTimeScale);
    
    return (tm == tms.mTime);
  }

  tm = tms.Convert(mTimeScale);
    
  return (mTime == tm);
}

bool SimulatorTime::operator==(unsigned tm) const
{
  assert(mTimeMsb == 0);

  return (mTime == tm);
}

bool SimulatorTime::operator!=(unsigned tm) const
{
  assert(mTimeMsb == 0);

  return (mTime != tm);
}

// End
