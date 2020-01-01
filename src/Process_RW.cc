// $RCSfile: Process_RW.cc,v $
// $Revision: 1.3 $

#include <assert.h>
#include <stdio.h>

#include "Process_RW.h"
//
#include "Enum.h"
#include "Global.h"
#include "InterfaceDesc_AXI.h"
#include "InterfaceDesc_RW.h"
#include "Timer.h"

extern int gDbg;
extern SimulatorTime *gSimTime;
M_ExternConstructDestructCountInt;

NameVal Process_RW_StateNV[] =
{
  Process_RW::eRW_WaitForTran, "RW_WaitForTran",
  Process_RW::eRW_Delay,       "RW_Delay",
  Process_RW::eRW_Addr,        "RW_Addr",
  Process_RW::eRW_WaitForReq,  "RW_WaitForReq",
  Process_RW::eRW_WaitForAck,  "RW_WaitForAck",
  Process_RW::eRW_AckDone,     "RW_AckDone",
};

Process_RW::Process_RW(const char* name, Module* parent_module, eRW_MasterOrSlave type) :
  Process           (name, parent_module),
  mRW_MasterOrSlave (type),
  mExecuteState     ((type==eRW_Master) ? eRW_WaitForTran : eRW_WaitForReq),
  mTimer            (0),
  mBit32            (32)
{
  mTimer = new Timer("RW_Timer", parent_module);

  M_UpdConstructCount(eType_Process_RW);
}

Process_RW::~Process_RW()
{
  // mTimer is deleted in 'parent_module'.

  M_UpdDestructCount(eType_Process_RW);
}

void Process_RW::ProcessExecute(ProcessExecuteReason& exec_reason)
{
  unsigned next_state;

  Interface& intf = *mpInterface;
  Bit req;

  if (intf[eRWIntf_Clk] == (unsigned)1) printf("ROB: Process_RW clk=1\n");
  else                                  printf("ROB: Process_RW clk=0\n");

  if (mRW_MasterOrSlave == eRW_Master) {
  //if (gDbg) printf("[%u] Dbg: %s state=%s\n",
  // gSimTime, intf.Name(), Enum2Str(Process_RW_StateNV, mExecuteState));

    switch (mExecuteState) {
    case eRW_WaitForTran:
      req = intf[eRWIntf_Req];

      intf[eRWIntf_Req ] = 1;
      intf[eRWIntf_Addr] = 0;
      intf[eRWIntf_Data] = 0;

      next_state = eRW_Delay;
      break;
    case eRW_Delay:
      printf("[%u] In Master:RW_Delay", gSimTime->TimeGet()), M_FL;
      mTimer->Start(50, eTimeScale_ns);
      Wait(mTimer);
      next_state = eRW_Addr;
      break;
    case eRW_Addr:
      printf("[%u] In Master:RW_Addr", gSimTime->TimeGet()), M_FL;
      intf[eRWIntf_Req ] = TRUE;
      intf[eRWIntf_Addr] = 100;
      intf[eRWIntf_Data] = 255;

      next_state = eRW_WaitForAck;
      break;
    case eRW_WaitForAck:
      if (intf[eRWIntf_Ack] == (unsigned)1) {
        intf[eRWIntf_Req ] = 0;

        next_state = eRW_WaitForTran;
      }
      else {
        intf[eRWIntf_Req ] = 0;

        next_state = mExecuteState; // ie, no change
      }

      break;
    }
  }
  else {
    // eRW_Slave:
    if (gDbg) printf("[%u] Dbg: %s state =%s", gSimTime->TimeGet(), intf.Name(), Enum2Str(Process_RW_StateNV, mExecuteState)), M_FL;

    switch (mExecuteState) {
    case eRW_WaitForReq:
      if (intf[eRWIntf_Req] == (unsigned)1) {
        printf("Addr:Data = ");
        mBit32 = intf[eRWIntf_Addr]; mBit32.Print("%s");
        printf(":");
        mBit32 = intf[eRWIntf_Data]; mBit32.Print("%s");
        printf("\nGot the Request!\n");

        intf[eRWIntf_Ack] = 1;

        next_state = eRW_AckDone;
      }
      else
        next_state = eRW_WaitForReq;

      break;
    case eRW_AckDone:
      intf[eRWIntf_Ack] = 0;

      next_state = eRW_WaitForReq;
    }
  }

  exec_reason.mExecutedOk = TRUE;

  mExecuteState = next_state;
}

// End
