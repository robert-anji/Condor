// $RCSfile: Interface.h,v $
// $Revision: 1.3 $

#ifndef InterfaceDescAxiMasApp
#define InterfaceDescAxiMasApp

enum eInterfaceDesc_AXI_MAS_APP {
  eAMA_CLK,
  eAMA_RESET,
  //
  eAMA_WR_DATA_REQ,
  eAMA_WR_ERR,
  eAMA_WR_RESP_VALID,
  eAMA_WR_ID,
  eAMA_WR_ID_IN,
  //
  eAMA_RD_DATA_VALID,
  eAMA_RD_DATA,
  eAMA_RD_ID,
  eAMA_RD_ID_IN,
  eAMA_RD_ERR,
  eAMA_RD_RESP_VALID,
  //
  eAMA_WR_REQ,
  eAMA_WR_START_ADDR,
  eAMA_WR_BURST_LEN,
  eAMA_WR_SIZE,
  eAMA_WR_ID_OUT,
  //
  eAMA_WR_DATA_VALID,
  eAMA_WR_DATA,
  //
  eAMA_RD_REQ,
  eAMA_RD_START_ADDR,
  eAMA_RD_BURST_LEN,
  eAMA_RD_SIZE,
  eAMA_RD_ID_OUT,
  //
  eAMA_Last
};

extern InterfaceDescription InterfaceDesc_AXI_MAS_APP [];

extern const char* const cAMA_CLK;
extern const char* const cAMA_RESET;
//
extern const char* const cAMA_WR_DATA_REQ;
extern const char* const cAMA_WR_ERR;
extern const char* const cAMA_WR_RESP_VAL;
extern const char* const cAMA_WR_ID;
//
extern const char* const cAMA_RD_DATA_VAL;
extern const char* const cAMA_RD_DATA;
extern const char* const cAMA_RD_ID;
extern const char* const cAMA_RD_ERR;
extern const char* const cAMA_RD_RESP_VAL;
//
extern const char* const cAMA_WR_REQ;
extern const char* const cAMA_WR_START_ADDR;
extern const char* const cAMA_WR_BURST_LEN;
extern const char* const cAMA_WR_SIZE;
extern const char* const cAMA_WR_ID;
//
extern const char* const cAMA_WR_DATA_VALID;
extern const char* const cAMA_WR_DATA;
//
extern const char* const cAMA_RD_REQ;
extern const char* const cAMA_RD_START_ADDR;
extern const char* const cAMA_RD_BURST_LEN;
extern const char* const cAMA_RD_SIZE;
extern const char* const cAMA_RD_I;

#endif

// End
