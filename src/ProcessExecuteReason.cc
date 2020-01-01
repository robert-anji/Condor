// $RCSfile: ProcessExecuteReason.cc,v $
// $Revision: 1.2 $

#include <assert.h>
#include <stdio.h>

#include "ProcessExecuteReason.h"
//
#include "Global.h"

M_ExternConstructDestructCountInt;

ProcessExecuteReason::ProcessExecuteReason() :
  mExecuteReason   (eExecuteReason_Invalid),
  mpReasonObject   (0),
  mExecutedOk      (FALSE)
{
  M_UpdConstructCount(eType_ProcessExecuteReason);
}

ProcessExecuteReason::~ProcessExecuteReason()
{
  M_UpdDestructCount(eType_ProcessExecuteReason);
}

void* ProcessExecuteReason::ReasonObjGet(eExecuteReasonObjType reason_obj_type)
{
  switch (reason_obj_type) {
  case eExecuteReasonObj_Clock:     assert(mExecuteReason == eExecuteReason_ClockPosedge
                                      ||   mExecuteReason == eExecuteReason_ClockNegedge);      break;
  case eExecuteReasonObj_Event:     assert(mExecuteReason == eExecuteReason_Event);             break;
  case eExecuteReasonObj_Interface: assert(mExecuteReason == eExecuteReason_InterfaceModified); break;
  case eExecuteReasonObj_Mailbox:   assert(mExecuteReason == eExecuteReason_MailboxPut
                                      ||   mExecuteReason == eExecuteReason_MailboxGet);        break;
  case eExecuteReasonObj_Timer:     assert(mExecuteReason == eExecuteReason_TimerDone);         break;
  case eExecuteReasonObj_Trigger:   assert(mExecuteReason == eExecuteReason_Trigger);           break;
  default:                          assert(0);
  //
  // Note: No entry for: eExecuteReason_Scheduler, we avoid returning a Scheduler object.
  }

  return mpReasonObject;
}

NameVal ExecuteReasonNV[] =
{
  eExecuteReason_Initial,            "Reason_Initial",
  eExecuteReason_ClockPosedge,       "Reason_ClockPosedge",
  eExecuteReason_ClockNegedge,       "Reason_ClockNegedge",
  eExecuteReason_Event,              "Reason_Event",
  eExecuteReason_InterfaceModified,  "Reason_InterfaceModified",
  eExecuteReason_MailboxPut,         "Reason_MailboxPut",
  eExecuteReason_MailboxGet,         "Reason_MailboxGet",
  eExecuteReason_SemaphoreGet,       "Reason_SemaphoreGet",
  eExecuteReason_SemaphorePut,       "Reason_SemaphorePut",
  eExecuteReason_Scheduler,          "Reason_Scheduler",
  eExecuteReason_TimerDone,          "Reason_TimerDone",
  eExecuteReason_Trigger,            "Reason_Trigger",
  eExecuteReason_Invalid,            "Reason_Invalid",
  eEnumInvalid,                      ""
};

// End
