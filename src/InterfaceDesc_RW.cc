// $RCSfile: Interface_RW.cc,v $
// $Revision: 1.1 $

#include "InterfaceDesc_RW.h"

// Note: Any new interface signals need to be also added in Interfaces.h .
const char* const cRWIntf_Clk  = "RWIntf_Clk";
const char* const cRWIntf_Req  = "RWIntf_Req";
const char* const cRWIntf_Addr = "RWIntf_Addr";
const char* const cRWIntf_Data = "RWIntf_Data";
const char* const cRWIntf_Ack  = "RWIntf_Ack";

InterfaceDescription InterfaceDesc_RW[] = {
  eRWIntf_Clk,  cRWIntf_Clk,   1, " ", eSignalType_Input,   "CLK",
  eRWIntf_Req,  cRWIntf_Req,   1, "R", eSignalType_Output,  "RW",
  eRWIntf_Addr, cRWIntf_Addr, 32, "R", eSignalType_Output,  "RW",
  eRWIntf_Data, cRWIntf_Data, 32, "R", eSignalType_Output,  "RW",
  eRWIntf_Ack , cRWIntf_Ack ,  1, "R", eSignalType_Input,   "RW",
  eRWIntf_END,  "RW",          0, " ", eSignalType_Invalid, ""
};

InterfaceDescription RWMaster_Interface[] = {
  eRWIntf_Clk,  cRWIntf_Clk,   1, " ", eSignalType_Input,   "CLK",
  eRWIntf_Req,  cRWIntf_Req,   1, "R", eSignalType_Output,  "MAS",
  eRWIntf_Addr, cRWIntf_Addr, 32, "R", eSignalType_Output,  "MAS",
  eRWIntf_Data, cRWIntf_Data, 32, "R", eSignalType_Output,  "MAS",
  eRWIntf_Ack , cRWIntf_Ack ,  1, "R", eSignalType_Input,   "MAS",
  eRWIntf_END,  "RW",          0, " ", eSignalType_Invalid, ""
};

InterfaceDescription RWSlave_Interface[] = {
  eRWIntf_Clk,  cRWIntf_Clk,   1, " ", eSignalType_Input,   "CLK",
  eRWIntf_Req,  cRWIntf_Req,   1, "R", eSignalType_Input,   "SLV",
  eRWIntf_Addr, cRWIntf_Addr, 32, "R", eSignalType_Input,   "SLV",
  eRWIntf_Data, cRWIntf_Data, 32, "R", eSignalType_Input,   "SLV",
  eRWIntf_Ack , cRWIntf_Ack ,  1, "R", eSignalType_Output,  "SLV",
  eRWIntf_END,  "RW",          0, " ", eSignalType_Invalid, ""
};

InterfaceDescription RWMonitor_Interface[] = {
  eRWIntf_Clk,  cRWIntf_Clk,   1, " ", eSignalType_Input,   "CLK",
  eRWIntf_Req,  cRWIntf_Req,   1, "R", eSignalType_Input,   "MON",
  eRWIntf_Addr, cRWIntf_Addr, 32, "R", eSignalType_Input,   "MON",
  eRWIntf_Data, cRWIntf_Data, 32, "R", eSignalType_Input,   "MON",
  eRWIntf_Ack , cRWIntf_Ack ,  1, "R", eSignalType_Input,   "MON",
  eRWIntf_END,  "RW",          0, " ", eSignalType_Invalid, ""
};

// End
