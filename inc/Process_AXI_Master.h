#ifndef PROCESS_AXI_MAS
#define PROCESS_AXI_MAS

#include "Bit.h"
#include "Process.h"
#include "Interface.h"

enum eMasterState {
  eMAS_WR_ADDR,
  eMAS_FIRST_WR_DATA,
  eMAS_WR_DATA,
  eMAS_WR_RESP,
  eMAS_RD_ADDR,
  eMAS_RD_DATA,
  eMAS_IDLE
};

enum eAXI_RESP {
  eOKAY        = 0,
  eEXOKAY      = 1,
  eSLVERR      = 2,
  eDECERR      = 3
};

class Process_AXI_Master : public Process
{
public:
  Process_AXI_Master(const char* name, Module* parent_module);
 ~Process_AXI_Master() {}

 Interface* mpInterface_AMA;

protected:
  virtual void ProcessExecute(ProcessExecuteReason& exec_reason);

private:
  Bit mas_state;
  Bit prev_mas_state;
  Bit wr_addr;
  Bit rd_addr;
  Bit wr_blen;
  Bit wr_start_blen;
  Bit rd_blen;
  Bit rd_start_blen;
  Bit wr_size;
  Bit rd_size;
  Bit wr_id;
  Bit rd_id;
  Bit wr_data_temp;
};

#endif
