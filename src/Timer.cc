// $RCSfile: Timer.cc,v $
// $Revision: 1.2 $

#include <assert.h>

#include "Timer.h"
//
#include "Global.h"
#include "Scheduler.h"
#include "SimulationUtilities.h"

extern Scheduler* gScheduler;
M_ExternConstructDestructCountInt;

Timer::Timer(const char* name, Module* module) :
  Process           (name, module),
  mTimeScale        (eTimeScale_ns),
  mTimerScheduled   (FALSE)
{
  mTimeStarted = 0;
  mTimeStopped = 0;

  mProcessState = eProcessState_NotYetStarted;

  M_UpdConstructCount(eType_Timer);
}

Timer::~Timer()
{
  M_UpdDestructCount(eType_Timer);
}

void Timer::ProcessExecute(ProcessExecuteReason& exec_reason)
{
  eProcessState next_state = mProcessState; // the default next state if no transition found

  switch (mProcessState) {
    case eProcessState_Running: {
      next_state = eProcessState_Completed;

      mTimerScheduled = FALSE;

      gScheduler->TimerDone(this);
    }
    break;
    //
    default:
      assert(0);
  }

  mProcessState = next_state;
}

bool Timer::Start(unsigned timeout, eTimeScale tm_scale, Process* process)
{
  mTimeout = timeout;

  return Start(mTimeout, tm_scale, process);
}

bool Timer::Start(const SimulatorTime& timeout, eTimeScale tm_scale, Process* process)
{
  assert (! mTimerScheduled); // cannot start if already started

  if (process) ProcessAdd(process);

  mTimeout.TimeScaleSet(tm_scale);
  mTimeout = timeout; // set this before scheduling timer below

  mTimeScale = tm_scale;

  gScheduler->ScheduleTimer(this);

  mProcessState   = eProcessState_Running;

  mTimeStarted    = gScheduler->SimulationTimeGet();
  mTimerScheduled = TRUE;
  mTimeStopped    = 0;

  return TRUE;
}

bool Timer::FlushTimer(Process** flushed_process)
{
  assert(mProcessState == eProcessState_Completed);

  // Flush 'waiting and subscribed' processes.
  while (mProcessList.First()) {
    Process* proc = mProcessList.Get();
    assert(proc);

    // Check if process is blocked or cannot be unblocked.
    if (gScheduler->ProcessIsBlocked(proc)
    &&  ! gScheduler->ProcessUnblock(proc, this, eSyncArg_None))
    {
      // A timer will finish only once so still delete the process.
      mProcessList.DeleteLinkThenNext();
      continue;
    }

    *flushed_process = proc;

    mProcessList.DeleteLinkThenIterationDone();

    return TRUE;
  }

  return FALSE;
}

SimulatorTime& Timer::TimeoutValue(eTimeScale tms)
{
  return mTimeout;
}

bool Timer::Restart()
{
  if (Stop()) {
    return Start(mTimeout, mTimeScale);
  }

  return FALSE;
}

bool Timer::Stop()
{
  // Do not allow a timer to stop when processes are watching or waiting.
  assert(mProcessList.Size() == 0);

  gScheduler->ProcessBlock(this, this);

  mTimerScheduled = FALSE;

  return TRUE;
}

SimulatorTime& Timer::TimeElapsed()
{
  mTimeElapsed = gScheduler->SimulationTimeGet() - mTimeStarted;

  return mTimeElapsed;
}

eProcessType Timer::ProcessType() const
{
  return eProcessType_Timer;
}

// End
