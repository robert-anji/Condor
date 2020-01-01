// $RCSfile: Interfaces.h,v $
// $Revision: 1.2 $

#ifndef InterfaceDescRW
#define InterfaceDescRW

#include "Interface.h"

enum eRWIntf {
  eRWIntf_Clk, 
  eRWIntf_Req, 
  eRWIntf_Addr,
  eRWIntf_Data,
  eRWIntf_Ack,
  eRWIntf_END
};

extern const char* const cRWIntf_Clk;
extern const char* const cRWIntf_Req;
extern const char* const cRWIntf_Addr;
extern const char* const cRWIntf_Data;
extern const char* const cRWIntf_Ack;

extern InterfaceDescription InterfaceDesc_RW        [];
extern InterfaceDescription InterfaceDesc_RWMaster  [];
extern InterfaceDescription InterfaceDesc_RWSlave   [];
extern InterfaceDescription InterfaceDesc_RWMonitor [];

#endif

// End
