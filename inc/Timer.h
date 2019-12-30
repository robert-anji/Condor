// $RCSfile: Timer.h,v $
// $Revision: 1.2 $

#ifndef TimerClass
#define TimerClass

#include "Process.h"
#include "Simulator.h"
#include "SimulatorTime.h"

class Interface;
class Module;

class Timer : public Process
{
  friend class Scheduler;

public:
  Timer(const char* name, Module* module);
 ~Timer();

  bool Start(unsigned             timeout, eTimeScale tm_scale, Process* p=0);
  bool Start(const SimulatorTime& timeout, eTimeScale tm_scale, Process* p=0);
  bool Stop();
  bool Restart();
  //
  SimulatorTime&  TimeoutValue(eTimeScale tms=eTimeScale_Default);
  SimulatorTime&  TimeStopped()   { return mTimeStopped; }
  SimulatorTime&  TimeElapsed();

  bool ProcessAdd(Process* p)      { mProcessList.Add(p); }

  virtual eProcessType ProcessType() const;

protected:
  virtual void ProcessExecute(ProcessExecuteReason& exec_reason);

private:
  SimulatorTime    mTimeout;
  SimulatorTime    mTimeElapsed;
  eTimeScale       mTimeScale;
  SimulatorTime    mTimeStarted;
  SimulatorTime    mTimeStopped;
  bool             mTimerScheduled;
  List<Process>    mProcessList;

  bool FlushTimer(Process** proc_blocked_on_timer); // for use bu Scheduler
};

#endif

// End
