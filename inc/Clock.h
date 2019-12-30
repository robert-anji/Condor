// $RCSfile: Clock.h,v $
// $Revision: 1.4 $

#ifndef ClockClass
#define ClockClass

#include "Bit.h"
#include "Process.h"
#include "ProcessExecuteReason.h"
#include "Simulator.h"
#include "SimulatorDefines.h"
#include "SimulatorTime.h"
#include "String.h"

class Interface;
class Module;

enum eClockPhase {
  eClockPhase_JustBeforePositiveEdge,
  eClockPhase_JustAfterPositiveEdge,
  eClockPhase_JustBeforeNegativeEdge,
  eClockPhase_JustAfterNegativeEdge,
  eClockPhase_Default = eClockPhase_JustAfterNegativeEdge,
  eClockPhase_Invalid
};

class Clock : public Process
{
  friend class Scheduler;

public:
  Clock(const char* name, Module* parent_module, unsigned half_period, eTimeScale s, eClockPhase phase, InterfaceDescription clk_intf[]);
  ~Clock();

  eSensitiveType    ClockEdgeGet()  const { return mClockEdge; }

  Clock& operator()(unsigned u); // experiment

  virtual eProcessType ProcessType() const;

protected:
  // Used by friend class Scheduler.
  virtual void    ProcessExecute(ProcessExecuteReason& exec_reason);
  bool            Start();
  bool            ProcessStateModify(eProcessState new_state);
  unsigned        SimTimeIncrForNextClockEdge(eSensitiveType sens_type);
  //
  unsigned        HalfPeriodGet()          const { return mHalfPeriod;              }

private:
  SimulatorTime   mClockStartTime;
  eTimeScale      mTimeScale;
  unsigned        mHalfPeriod;
  unsigned        mHalfPeriodScaled; // scale according to Scheduler timescale
  eClockPhase     mClockPhaseStart;
  eClockPhase     mClockPhase;
  eSensitiveType  mClockEdge;
  Bit             mBit;
  bool            mTimeHasAdvanced;
  bool            mClockScheduleSuspend;
  List<Process>   mConnectedProcessList;
};

#endif

// End
