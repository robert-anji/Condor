// $RCSfile: Event.h,v $
// $Revision: 1.3 $

#ifndef EventClass
#define EventClass

#include "SimulatorDefines.h"
#include "String.h"
#include "List.h"

class Process;

class Event
{
friend class Scheduler;

public:
  Event(const char* name);
 ~Event();

  void Set(); // maybe Send()      is a better function name?
  void Get(); // maybe Subscribe() is a better function name?

  // Access functions:
  const char* Name() const { return (const char*)mName; }

private:
  String        mName;
  List<Process> mProcessesWaitingForEventList;

  Process* Flush(); // for Scheduler
};

#endif

// End
