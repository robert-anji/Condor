// $RCSfile: Process.cc,v $
// $Revision: 1.4 $
// 1. Added member mProcessName.

#include <assert.h>
#include <stdio.h>

#include "Process.h"
//
#include "Global.h"
#include "Enum.h"
#include "Interface.h"
#include "MailboxBase.h"
#include "Module.h"
#include "Scheduler.h"
#include "Semaphore.h"
#include "Timer.h"

extern Scheduler* gScheduler;
M_ExternConstructDestructCountInt;

Process::Process(const char* name, Module* parent_module) :
  mProcessName           (name),
  mpModule               (parent_module),
  mpInterface            (0),
  mProcessState          (eProcessState_Running),
  mpProcessExecFnPtr     (0),
  mpRecord               (0),
  mpBlockingAgentPending (0),
  mBlockAgentArgPending  (0),
  mDeltaCycle            (0),
  mUnblockedInCurrentDeltaCycle(FALSE),
  mHasExecutedInitial    (FALSE)
{
  mpModule->ProcessAdd(this);

  M_UpdConstructCount(eType_Process);
}

Process::Process(const char* name, Module* parent_module, ProcessExecFnPtr fn_ptr) :
  mProcessName           (name),
  mpModule               (parent_module),
  mpInterface            (0),
  mProcessState          (eProcessState_Running),
  mpProcessExecFnPtr     (fn_ptr),
  mpRecord               (0),
  mpBlockingAgentPending (0),
  mBlockAgentArgPending  (0),
  mDeltaCycle            (0),
  mUnblockedInCurrentDeltaCycle(FALSE),
  mHasExecutedInitial    (FALSE)
{
  mpModule->ProcessAdd(this);

  M_UpdConstructCount(eType_Process);
}

Process::Process(const char* name, Module* parent_module, Interface* intf) :
  mProcessName           (name),
  mpModule               (parent_module),
  mpInterface            (intf),
  mProcessState          (eProcessState_Running),
  mpProcessExecFnPtr     (0),
  mpRecord               (0),
  mpBlockingAgentPending (0),
  mBlockAgentArgPending  (0),
  mDeltaCycle            (0),
  mUnblockedInCurrentDeltaCycle(FALSE),
  mHasExecutedInitial    (FALSE)
{
  mpModule->ProcessAdd(this);

  intf->SetOwningProcess(this);

  M_UpdConstructCount(eType_Process);
}

Process::~Process()
{
  if (mpInterface) { delete mpInterface; mpInterface = 0; }

  M_ListDeleteAll(mSensitiveInfoList, SensitiveInfo);

  M_UpdDestructCount(eType_Process);
}

// Just remember which signal and edge the process is sensitive to. This info
// will be used when the process first gets scheduled at which time this signal
// should be connected to something (clock or an output signal).
bool Process::ProcessIsSensitiveTo(const char* signal_name, eSensitiveType sens_type)
{
  assert(mpInterface);
  Signal* signal = mpInterface->SignalGet(signal_name);
  assert(signal);

  SensitiveInfo* info = new SensitiveInfo;
  assert(info);

  // Dont add the entry if it already exists.
  for (M_ListIterateAll(mSensitiveInfoList)) {
    SensitiveInfo* sensitive_edge_info = mSensitiveInfoList.Get();
    assert(sensitive_edge_info);

    if (sensitive_edge_info->mSignalName    == signal_name
    &&  sensitive_edge_info->mSensitiveType == sens_type)
    {
      printf("Warning: ignoring duplicate sensitivity for signal %s sens_type %s",
       signal_name, Enum2Str(SignalEdgeNV, sens_type));

      return TRUE; // warning, not error, so return true
    }
  }

  // Initialize the new proces,signal_name,sens_type combination.
  info->mpProcess      = this;
  info->mSignalName    = signal_name;
  info->mSensitiveType = sens_type;

  // Add the new proces,signal_name,sens_type combination.
  // The main use is in Signal where the input signal change can cause the
  // process to be triggered, we also keep it here for convenience to know
  // all the sensitivities for the Process.
  bool result1 = signal->mSensitiveInfoList.Add(info);
  bool result2 =   this->mSensitiveInfoList.Add(info);

  return (result1 && result2);
}

bool Process::InterfaceSet(Interface* intf)
{
  // Allow an interface to be added only if it doesnt yet exist.
  if (mpInterface == 0) { mpInterface = intf; mpInterface->SetOwningProcess(this); }
  else                  assert(0);
}

void Process::Wait(Timer* timer)
{
  timer->ProcessAdd(this);

  gScheduler->ProcessBlock(this, timer);
}

void Process::Wait(Event* event)
{
  gScheduler->ProcessBlock(this, event);
}

void Process::Wait(Semaphore* sem, eSyncArg sync_arg)
{
  gScheduler->ProcessBlock(this, sem, sync_arg);
}

void Process::Wait(MailboxBase* mbox, eSyncArg sync_arg)
{
  gScheduler->ProcessBlock(this, mbox, sync_arg);
}

void Process::ProcessExecute(ProcessExecuteReason& exec_reason)
{
  const char* tm_str = gScheduler->SimulationTimeStrGet();

  printf("[%s] %s: Executing", tm_str, (const char*)mProcessName); M_FL;

  if (mpProcessExecFnPtr) (*mpProcessExecFnPtr)(exec_reason, this);

  exec_reason.mExecutedOk = TRUE;
}

bool Process::ProcessStateModify(eProcessState new_state)
{
  // If this is supported then the derived class needs to implement this.
  // If the derived class does not override this function then error out.
  assert(0);

  return FALSE;
}

eProcessType Process::ProcessType() const
{
  if      (mpInterface == 0)              return eProcessType_Software;
  else if (mpInterface->IsSynchronous())  return eProcessType_Synchronous;
  else                                    return eProcessType_Asynchronous;

  // Note: Timer & Clock are derived classes from Process and override this function.
}

// End
