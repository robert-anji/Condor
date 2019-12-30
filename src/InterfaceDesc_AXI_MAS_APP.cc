// $RCSfile: Interface.h,v $
// $Revision: 1.3 $

#include "Interface.h"

#include "InterfaceDesc_AXI_MAS_APP.h"


// AXI Interface Description
InterfaceDescription InterfaceDesc_AXI_MAS_APP[] = {
  eAMA_CLK,            cAMA_CLK,             " ", 1, eSignalType_Input, "CLK",
  eAMA_RESET,          cAMA_RESET,           " ", 1, eSignalType_Input, "RESET",
  //
  eAMA_WR_DATA_REQ,    cAMA_WR_DATA_REQ,     "R", 1, eSignalType_Input, "WR_DATA_REQ",
  eAMA_WR_ERR,         cAMA_WR_ERR,          "R", 1, eSignalType_Input, "WR_ERR",
  eAMA_WR_RESP_VAL,    cAMA_WR_RESP_VAL,     "R", 1, eSignalType_Input, "WR_RESP_VAL",
  eAMA_WR_ID,          cAMA_WR_ID,           "R", 1, eSignalType_Input, "WR_ID",
  //
  eAMA_RD_DATA_VAL,    cAMA_RD_DATA_VAL,     "R", 1, eSignalType_Input, "RD_DATA_VAL",
  eAMA_RD_DATA,        cAMA_RD_DATA,         "R", 1, eSignalType_Input, "RD_DATA",
  eAMA_RD_ID,          cAMA_RD_ID,           "R", 1, eSignalType_Input, "RD_ID",
  eAMA_RD_ERR,         cAMA_RD_ERR,          "R", 1, eSignalType_Input, "RD_ERR",
  eAMA_RD_RESP_VAL,    cAMA_RD_RESP_VAL,     "R", 1, eSignalType_Input, "RD_RESP_VAL",
  //
  eAMA_WR_REQ,         cAMA_WR_REQ,          "R", 1, eSignalType_Input, "WR_REQ",
  eAMA_WR_START_ADDR,  cAMA_WR_START_ADDR,   "R", 1, eSignalType_Input, "WR_START_ADDR",
  eAMA_WR_BURST_LEN,   cAMA_WR_BURST_LEN,    "R", 1, eSignalType_Input, "WR_BURST_LEN",
  eAMA_WR_SIZE,        cAMA_WR_SIZE,         "R", 1, eSignalType_Input, "WR_SIZE",
  eAMA_WR_ID,          cAMA_WR_ID,           "R", 1, eSignalType_Input, "WR_ID",
  //
  eAMA_WR_DATA_VALID,  cAMA_WR_DATA_VALID,   "R", 1, eSignalType_Input, "WR_DATA_VALID",
  eAMA_WR_DATA,        cAMA_WR_DATA,         "R", 1, eSignalType_Input, "WR_DATA",
  //
  eAMA_RD_REQ,         cAMA_RD_REQ,          "R", 1, eSignalType_Input, "RD_REQ",
  eAMA_RD_START_ADDR,  cAMA_RD_START_ADDR,   "R", 1, eSignalType_Input, "RD_START_ADDR",
  eAMA_RD_BURST_LEN,   cAMA_RD_BURST_LEN,    "R", 1, eSignalType_Input, "RD_BURST_LEN",
  eAMA_RD_SIZE,        cAMA_RD_SIZE,         "R", 1, eSignalType_Input, "RD_SIZE",
  eAMA_RD_ID,          cAMA_RD_ID,           "R", 1, eSignalType_Input, "RD_ID",
  eAMA_End             "AMA",                " ", 0, eSignalType_Invalid, "",
};

// AXI Master Applicatipn Signal names
// Clock and Reset
const char* const cAMA_CLK             = "AMA_CLK";
const char* const cAMA_RESET           = "AMA_RESET";
//
const char* const cAMA_WR_DATA_REQ     = "AMA_WR_DATA_REQ";
const char* const cAMA_WR_ERR          = "AMA_WR_ERR";
const char* const cAMA_WR_RESP_VAL     = "AMA_WR_RESP_VAL";
const char* const cAMA_WR_ID           = "AMA_WR_ID";
//
const char* const cAMA_RD_DATA_VAL     = "AMA_RD_DATA_VAL";
const char* const cAMA_RD_DATA         = "AMA_RD_DATA";
const char* const cAMA_RD_ID           = "AMA_RD_ID";
const char* const cAMA_RD_ERR          = "AMA_RD_ERR";
const char* const cAMA_RD_RESP_VAL     = "AMA_RD_RESP_VAL";
//
const char* const cAMA_WR_REQ          = "AMA_WR_REQ";
const char* const cAMA_WR_START_ADDR   = "AMA_WR_START_ADDR";
const char* const cAMA_WR_BURST_LEN    = "AMA_WR_BURST_LEN";
const char* const cAMA_WR_SIZE         = "AMA_WR_SIZE";
const char* const cAMA_WR_ID           = "AMA_WR_ID";
//
const char* const cAMA_WR_DATA_VALID   = "AMA_WR_DATA_VALID";
const char* const cAMA_WR_DATA         = "AMA_WR_DATA";
//
const char* const cAMA_RD_REQ          = "AMA_RD_REQ";
const char* const cAMA_RD_START_ADDR   = "AMA_RD_START_ADDR";
const char* const cAMA_RD_BURST_LEN    = "AMA_RD_BURST_LEN";
const char* const cAMA_RD_SIZE         = "AMA_RD_SIZE";
const char* const cAMA_RD_I            = "AMA_RD_I";

// End
