// $RCSfile: Interface.h,v $
// $Revision: 1.3 $

#ifndef InterfaceDescAXI
#define InterfaceDescAXI

enum eInterfaceDesc_AXI {
  eAXI_ACLK,
  eAXI_ARESET,
  // Write Address Channel.
  eAXI_AWID,
  eAXI_AWADDR,
  eAXI_AWLEN,
  eAXI_AWSIZE,
  eAXI_AWBURST,
  eAXI_AWLOCK,
  eAXI_AWCACHE,
  eAXI_AWPROT,
  eAXI_AWVALID,
  eAXI_AWREADY,
  // Write Data Channel.
  eAXI_WID,
  eAXI_WDATA,
  eAXI_WSTRB,
  eAXI_WLAST,
  eAXI_WVALID,
  eAXI_WREADY,
  // Write Response Channel.
  eAXI_BID,
  eAXI_BRESP,
  eAXI_BVALID,
  eAXI_BREADY,
  // Read Address Channel.
  eAXI_ARID,
  eAXI_ARADDR,
  eAXI_ARLEN,
  eAXI_ARSIZE,
  eAXI_ARBURST,
  eAXI_ARLOCK,
  eAXI_ARCACHE,
  eAXI_ARPROT,
  eAXI_ARVALID,
  eAXI_ARREADY,
  // Read Data Channel.
  eAXI_RID,
  eAXI_RDATA,
  eAXI_RRESP,
  eAXI_RLAST,
  eAXI_RVALID,
  eAXI_RREADY,
  // Placeholder for 'end'
  eAXI_DescEnd
};

extern InterfaceDescription InterfaceDesc_AXI [];

// AXI Signal names // +++
// Reset and Clock.
extern const char* const cAXI_ACLK;
extern const char* const cAXI_ARESET;
// Write Address Channel.
extern const char* const cAXI_AWID;
extern const char* const cAXI_AWADDR;
extern const char* const cAXI_AWLEN;
extern const char* const cAXI_AWSIZE;
extern const char* const cAXI_AWBURST;
extern const char* const cAXI_AWLOCK;
extern const char* const cAXI_AWCACHE;
extern const char* const cAXI_AWPROT;
extern const char* const cAXI_AWVALID;
extern const char* const cAXI_AWREADY;
// Write Data Channel.
extern const char* const cAXI_WID;
extern const char* const cAXI_WDATA;
extern const char* const cAXI_WSTRB;
extern const char* const cAXI_WLAST;
extern const char* const cAXI_WVALID;
extern const char* const cAXI_WREADY;
// Write Response Channel.
extern const char* const cAXI_BID;
extern const char* const cAXI_BRESP;
extern const char* const cAXI_BVALID;
extern const char* const cAXI_BREADY;
// Read Address Channel.
extern const char* const cAXI_ARID;
extern const char* const cAXI_ARADDR;
extern const char* const cAXI_ARLEN;
extern const char* const cAXI_ARSIZE;
extern const char* const cAXI_ARBURST;
extern const char* const cAXI_ARLOCK;
extern const char* const cAXI_ARCACHE;
extern const char* const cAXI_ARPROT;
extern const char* const cAXI_ARVALID;
extern const char* const cAXI_ARREADY;
// Read Data Channel.
extern const char* const cAXI_RID;
extern const char* const cAXI_RDATA;
extern const char* const cAXI_RRESP;
extern const char* const cAXI_RLAST;
extern const char* const cAXI_RVALID;
extern const char* const cAXI_RREADY;
// ---

#endif

// End
