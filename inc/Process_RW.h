// $RCSfile: Process_RW.h,v $
// $Revision: 1.2 $

#ifndef Process_RWClass
#define Process_RWClass

#include "Bit.h"
#include "Interface.h"

#include "Process.h"
#include "ProcessExecuteReason.h"

class Timer;

class Process_RW : public Process
{
public:
  enum eRW_MasterOrSlave { eRW_Master, eRW_Slave };
  //
  enum eExecuteState     { eRW_WaitForTran, eRW_Addr, eRW_Delay,
                           eRW_WaitForReq,  eRW_WaitForAck, eRW_AckDone };

  Process_RW(const char* name, Module* parent_module, eRW_MasterOrSlave type);
  ~Process_RW();

protected:
  virtual void ProcessExecute(ProcessExecuteReason& exec_reason);

private:
  unsigned            mExecuteState;
  eRW_MasterOrSlave   mRW_MasterOrSlave;
  Timer*              mTimer; // candidate for impl
  Bit                 mBit32; // candidate for impl
};

#endif

// End
