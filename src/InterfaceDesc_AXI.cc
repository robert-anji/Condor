
#include "Interface.h"

#include "InterfaceDesc_AXI.h"


// AXI Interface Description
InterfaceDescription InterfaceDesc_AXI[] = {
// position        name      width flags     direction      group
  eAXI_ACLK,    cAXI_ACLK,      1, " ", eSignalType_Input,  "CLK",
  eAXI_ARESET,  cAXI_ARESET,    1, " ", eSignalType_Input,  "RESET",
  // Write Address Channel.
  eAXI_AWID,    cAXI_AWID,      4, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWADDR,  cAXI_AWADDR,   32, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWLEN,   cAXI_AWLEN,     4, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWSIZE,  cAXI_AWSIZE,    3, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWBURST, cAXI_AWBURST,   2, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWLOCK,  cAXI_AWLOCK,    2, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWCACHE, cAXI_AWCACHE,   4, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWPROT,  cAXI_AWPROT,    3, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWVALID, cAXI_AWVALID,   1, "R", eSignalType_Output, "WrAddrChn",
  eAXI_AWREADY, cAXI_AWREADY,   1, "R", eSignalType_Input,  "WrAddrChn",
  // Write Data Channel.
  eAXI_WID,     cAXI_WID,       4, "R", eSignalType_Output, "WrDataChn",
  eAXI_WDATA,   cAXI_WDATA,    32, "R", eSignalType_Output, "WrDataChn",
  eAXI_WSTRB,   cAXI_WSTRB,     4, "R", eSignalType_Output, "WrDataChn",
  eAXI_WLAST,   cAXI_WLAST,     1, "R", eSignalType_Output, "WrDataChn",
  eAXI_WVALID,  cAXI_WVALID,    1, "R", eSignalType_Output, "WrDataChn",
  eAXI_WREADY,  cAXI_WREADY,    1, "R", eSignalType_Input,  "WrDataChn",
  // Write Response Channel.
  eAXI_BID,     cAXI_BID,       4, "R", eSignalType_Input,  "WrRespChn",
  eAXI_BRESP,   cAXI_BRESP,     2, "R", eSignalType_Input,  "WrRespChn",
  eAXI_BVALID,  cAXI_BVALID,    1, "R", eSignalType_Input,  "WrRespChn",
  eAXI_BREADY,  cAXI_BREADY,    1, "R", eSignalType_Output, "WrRespChn",
  // Read Address Channel.
  eAXI_ARID,    cAXI_ARID,      4, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARADDR,  cAXI_ARADDR,   32, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARLEN,   cAXI_ARLEN,     4, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARSIZE,  cAXI_ARSIZE,    3, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARBURST, cAXI_ARBURST,   2, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARLOCK,  cAXI_ARLOCK,    2, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARCACHE, cAXI_ARCACHE,   4, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARPROT,  cAXI_ARPROT,    3, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARVALID, cAXI_ARVALID,   1, "R", eSignalType_Output, "RdAddrChn",
  eAXI_ARREADY, cAXI_ARREADY,   1, "R", eSignalType_Input,  "RdAddrChn",
  // Read Data Channel.
  eAXI_RID,     cAXI_RID,       4, "R", eSignalType_Input,  "RdDataChn",
  eAXI_RDATA,   cAXI_RDATA,    32, "R", eSignalType_Input,  "RdDataChn",
  eAXI_RRESP,   cAXI_RRESP,     2, "R", eSignalType_Input,  "RdDataChn",
  eAXI_RLAST,   cAXI_RLAST,     1, "R", eSignalType_Input,  "RdDataChn",
  eAXI_RVALID,  cAXI_RVALID,    1, "R", eSignalType_Input,  "RdDataChn",
  eAXI_RREADY,  cAXI_RREADY,    1, "R", eSignalType_Output, "RdDataChn",
  // Interface name is placed in the last entry.
  eAXI_DescEnd, "AXI",          0, " ", eSignalType_Invalid,""
};


// AXI Signal names
// Clock and Reset
const char* const cAXI_ACLK     = "ACLK";
const char* const cAXI_ARESET   = "ARESET";
// Write Address Channel.
const char* const cAXI_AWID     = "AWID";
const char* const cAXI_AWADDR   = "AWADDR";
const char* const cAXI_AWLEN    = "AWLEN";
const char* const cAXI_AWSIZE   = "AWSIZE";
const char* const cAXI_AWBURST  = "AWBURST";
const char* const cAXI_AWLOCK   = "AWLOCK";
const char* const cAXI_AWCACHE  = "AWCACHE";
const char* const cAXI_AWPROT   = "AWPROT";
const char* const cAXI_AWVALID  = "AWVALID";
const char* const cAXI_AWREADY  = "AWREADY";
// Write Data Channel.
const char* const cAXI_WID      = "AXI_WID";
const char* const cAXI_WDATA    = "AXI_WDATA";
const char* const cAXI_WSTRB    = "AXI_WSTRB";
const char* const cAXI_WLAST    = "AXI_WLAST";
const char* const cAXI_WVALID   = "AXI_WVALID";
const char* const cAXI_WREADY   = "AXI_WREADY";
// Write Response Channel.
const char* const cAXI_BID      = "AXI_BID";
const char* const cAXI_BRESP    = "AXI_BRESP";
const char* const cAXI_BVALID   = "AXI_BVALID";
const char* const cAXI_BREADY   = "AXI_BREADY";
// Read Address Channel.
const char* const cAXI_ARID     = "AXI_ARID";
const char* const cAXI_ARADDR   = "AXI_ARADDR";
const char* const cAXI_ARLEN    = "AXI_ARLEN";
const char* const cAXI_ARSIZE   = "AXI_ARSIZE";
const char* const cAXI_ARBURST  = "AXI_ARBURST";
const char* const cAXI_ARLOCK   = "AXI_ARLOCK";
const char* const cAXI_ARCACHE  = "AXI_ARCACHE";
const char* const cAXI_ARPROT   = "AXI_ARPROT";
const char* const cAXI_ARVALID  = "AXI_ARVALID";
const char* const cAXI_ARREADY  = "AXI_ARREADY";
// Read Data Channel.
const char* const cAXI_RID      = "AXI_RID";
const char* const cAXI_RDATA    = "AXI_RDATA";
const char* const cAXI_RRESP    = "AXI_RRESP";
const char* const cAXI_RLAST    = "AXI_RLAST";
const char* const cAXI_RVALID   = "AXI_RVALID";
const char* const cAXI_RREADY   = "AXI_RREADY";

// End
