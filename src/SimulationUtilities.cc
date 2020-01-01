// $RCSfile: SeedHelper.cc,v $
// $Revision: 1.1 $

#include "Scheduler.h"

extern Scheduler* gScheduler;
extern Simulator* gSimulator;

unsigned SeedGet()
{
  return gSimulator->SeedGet();
}

Process* GetCurrentProcess()
{
  return gScheduler->GetCurrentProcess();
}

void Wait(Timer* timer)
{
  gScheduler->ProcessBlock(GetCurrentProcess(), timer);
}

void Wait(Event* event)
{
  gScheduler->ProcessBlock(GetCurrentProcess(), event);
}

void Wait(MailboxBase* mbox, eSyncArg sync_arg)
{
  gScheduler->ProcessBlock(GetCurrentProcess(), mbox, sync_arg);
}

void Wait(Semaphore* sem, eSyncArg sync_arg)
{
  gScheduler->ProcessBlock(GetCurrentProcess(), sem, sync_arg);
}

eTimeScale SchedulerTimeScaleGet()
{
  return gScheduler->SchedulerTimeScaleGet();
}

// End
