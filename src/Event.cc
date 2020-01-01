// $RCSfile: Event.cc,v $
// $Revision: 1.4 $

#include "Event.h"
//
#include "Global.h"
#include "Scheduler.h"
#include "SimulationUtilities.h"

extern Scheduler* gScheduler;
M_ExternConstructDestructCountInt;


Event::Event(const char* name) :
  mName(name)
{
  M_UpdConstructCount(eType_Event);
}

Event::~Event()
{
  mProcessesWaitingForEventList.DeleteLinkAll();

  M_UpdDestructCount(eType_Event);
}

void Event::Set()
{
  gScheduler->ScheduleEvent(this);
}

void Event::Get()
{
  mProcessesWaitingForEventList.Add(GetCurrentProcess());
}

Process* Event::Flush()
{
  Process* process = 0;

  while (mProcessesWaitingForEventList.First()) {
    process = mProcessesWaitingForEventList.Get();
    assert(process);

    if (gScheduler->ProcessIsBlocked(process)
    &&  ! gScheduler->ProcessUnblock(process, this, eSyncArg_None))
    {
      // 'process' will miss the event but we keep it around to try again.
      process = 0;
      continue;
    }

    mProcessesWaitingForEventList.DeleteLinkThenIterationDone();
  }

  return process;
}

// End
