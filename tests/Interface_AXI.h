// $RCSfile: Interface_AXI.h,v $
// $Revision: 1.2 $

#ifndef InterfaceAXI
#define InterfaceAXI

#include "Interface.h"

// Refer to axi_spec for AXI signals:
// - write_addr_chn: 2.2       Write address channel signals

namespace AXI {

enum eAXI_Wr_Intf {
  eAWID,
  eAWADDR,
  eAWLEN,
  eAWSIZE,
  eAWBURST,
  eAWLOCK,
  eAWCACHE,
  eAWPROT,
  eAWVALID,
  eAWREADY,
  eAXI_Wr_Intf_END
};

}

extern const char* const cWrIntf_Req;

extern InterfaceDescription RWMaster_Interface   [];
extern InterfaceDescription RWSlave_Interface    [];
extern InterfaceDescription RWMonitor_Interface  [];

#endif

// End
