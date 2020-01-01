// $RCSfile: Process.h,v $
// $Revision: 1.7 $

#ifndef ProcessClass
#define ProcessClass

#include <assert.h>

#include "List.h"
#include "Interface.h"
#include "ProcessExecuteReason.h"
#include "Semaphore.h"
#include "SimulatorDefines.h"
#include "Simulator.h"
#include "String.h"

enum eProcessState {
  eProcessState_NotYetStarted,
  eProcessState_Running,
  eProcessState_Suspended,
  eProcessState_Stopped,
  eProcessState_Completed
};

enum eProcessType {
  eProcessType_Clock,
  eProcessType_Synchronous,
  eProcessType_Software,
  eProcessType_Timer,
  eProcessType_Asynchronous,
  eProcessType_Invalid
};

typedef void (*ProcessExecFunctionPtr)(ProcessExecuteReason &proc_exec_reason, Process* proc);
typedef        ProcessExecFunctionPtr  ProcessExecFnPtr;

class MailboxBase;
class Event;
class Module;
class Record;
class Timer;

class Process
{
  friend class Module;
  friend class Scheduler;
  friend class Vcd;

public:
  Process(const char* name, Module* parent_module);
  Process(const char* name, Module* parent_module, ProcessExecFnPtr fn_ptr);
  Process(const char* name, Module* parent_module, Interface* intf);
  //
  virtual ~Process();

  bool ProcessIsSensitiveTo(const char* signal_name, eSensitiveType edge);
//bool ProcessIsSensitiveTo(Event* evt); // TBD. where event is timer, mbox or notify etc.

  Interface* InterfaceGet() { return mpInterface; }
  bool       InterfaceSet(Interface* intf); // only one invocation allowed

  // Wait() functions.
  void Wait(Timer* timer);
  void Wait(Event* event);
  void Wait(Semaphore*   sem,  eSyncArg sync_arg);
  void Wait(MailboxBase* mbox, eSyncArg sync_arg);
  void Wait(unsigned tm, eTimeScale ts);

  // Query functions.
  const char* Name()            const { return (const char*)mProcessName; }
  Module*     GetOwningModule() const { return mpModule;                  }
  //
  virtual bool          ProcessStateModify(eProcessState state);
  virtual eProcessState ProcessStateGet() { return mProcessState; }
  virtual eProcessType  ProcessType() const;

protected:
  String          mProcessName;
  Interface*      mpInterface;
  Module*         mpModule;
  eProcessState   mProcessState;
  //
  ProcessExecFnPtr mpProcessExecFnPtr;
  //
  List<SensitiveInfo> mSensitiveInfoList;

  // Derived class should implement the Execute() function.
  virtual void ProcessExecute(ProcessExecuteReason& exec_reason);

  // For convenience the Scheduler stores some Process-Blocked info here.
  void*    mpBlockingAgentPending;
  unsigned mBlockAgentArgPending;

private:
  Record*  mpRecord; // Scheduler use only, keep record for each process handy
  unsigned mDeltaCycle; // delta cycle when process was last executed
  bool     mUnblockedInCurrentDeltaCycle; // Scheduler use only, avoid races
  bool     mHasExecutedInitial; // Scheduler use only, ensure invoked once
};

#endif

// End
