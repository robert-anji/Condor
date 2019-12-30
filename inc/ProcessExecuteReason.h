// $RCSfile: ProcessExecuteReason.h,v $
// $Revision: 1.3 $

#ifndef ProcessExecuteReasonClass
#define ProcessExecuteReasonClass

#include "Enum.h"

enum eExecuteReasonObjType {
  eExecuteReasonObj_Clock,
  eExecuteReasonObj_Interface,
  eExecuteReasonObj_Mailbox,
  eExecuteReasonObj_Event,
  eExecuteReasonObj_Timer,
  eExecuteReasonObj_Trigger
};

enum eExecuteReason
{
  eExecuteReason_Initial,
  eExecuteReason_ClockPosedge,
  eExecuteReason_ClockNegedge,
  eExecuteReason_Event,
  eExecuteReason_InterfaceModified,
  eExecuteReason_MailboxPut,
  eExecuteReason_MailboxGet,
  eExecuteReason_SemaphoreGet,
  eExecuteReason_SemaphorePut,
  eExecuteReason_Scheduler,
  eExecuteReason_TimerDone,
  eExecuteReason_Trigger,
  eExecuteReason_Invalid
};

extern NameVal ExecuteReasonNV[];

class ProcessExecuteReason
{
  friend class Scheduler;

public:
  ProcessExecuteReason();
 ~ProcessExecuteReason();

  void* ReasonObjGet(eExecuteReasonObjType reason_obj_type);

  bool mExecutedOk; // TBD: check if we really need this, Process may forget to set this
  eExecuteReason ReasonGet() { return mExecuteReason; }

private:
  eExecuteReason mExecuteReason;
  void*          mpReasonObject;
};

#endif

// End
