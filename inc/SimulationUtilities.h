// $RCSfile: SeedHelper.h,v $
// $Revision: 1.1 $

#ifndef SIMULATIONUTILITIES_DECL
#define SIMULATIONUTILITIES_DECL

#include "SimulatorDefines.h"

class Timer;
class MailboxBase;
class Process;
class Semaphore;

unsigned SeedGet(void);

Process* GetCurrentProcess();
//
void Wait(Timer*       timer);
void Wait(Event*       event);
void Wait(Semaphore*   sem,  eSyncArg sync_arg);
void Wait(MailboxBase* mbox, eSyncArg sync_arg);
//
eTimeScale SchedulerTimeScaleGet();

#endif

// End
