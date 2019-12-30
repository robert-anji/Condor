// $RCSfile: MailboxBase.h,v $
// $Revision: 1.1 $

#ifndef MailboxBaseClass
#define MailboxBaseClass

#include "Global.h"
#include "List.h"
#include "SimulatorDefines.h"
#include "String.h"

// Forward declaration.
class Process;

class MailboxBase
{
friend class Scheduler;

public:
  MailboxBase(const char* name);
 ~MailboxBase();

  const char* Name() { return mName; }

protected:
  List<Process> mProcessWaitingOnPutList;
  List<Process> mProcessWaitingOnGetList;
  List<void>    mMboxElmWaitingOnPutList;

private:
  String        mName;

  bool FlushPutsAndGets(Process** proc_put, Process** proc_get, void** elm); // for Scheduler
};

#endif
