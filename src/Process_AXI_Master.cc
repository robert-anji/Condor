#include "Process_AXI_Master.h"

#include "InterfaceDesc_AXI.h"
#include "InterfaceDesc_AXI_MAS_APP.h"

//extern FILE* gFp;

Process_AXI_Master::Process_AXI_Master(const char* name, Module* parent_module) :
  Process(name, parent_module),
  mas_state       (4),
  prev_mas_state  (4),
  wr_addr         (32),
  rd_addr         (32),
  wr_blen         (4),
  wr_start_blen   (4),
  rd_blen         (4),
  rd_start_blen   (4),
  wr_size         (3),
  rd_size         (3),
  wr_id           (4),
  rd_id           (4),
  wr_data_temp    (32)
{
}

void Process_AXI_Master::ProcessExecute(ProcessExecuteReason& exec_reason)
{
  Interface& intf     = *mpInterface;
  Interface& intf_ama = *mpInterface_AMA;

  if (mas_state != prev_mas_state)
  //fprintf(gFp, "[%s] MAS: state=%d\n", SIM_TM_STR, (unsigned)mas_state);

  if (! intf[eAXI_ARESET]) {
    mas_state      = eMAS_IDLE;
    prev_mas_state = eMAS_IDLE;
  }
  else {
    prev_mas_state = mas_state;

    switch ((int)mas_state) {
    case eMAS_IDLE:
      intf[eAXI_AWID]    = 0;
      intf[eAXI_AWADDR]  = 0;
      intf[eAXI_AWLEN]   = 0;
      intf[eAXI_AWSIZE]  = 0;
      intf[eAXI_AWBURST] = 0;
      intf[eAXI_AWVALID] = 0;
      intf[eAXI_BREADY]  = 0;
      intf[eAXI_WID]     = 0;
      intf[eAXI_WDATA]   = 0;
      intf[eAXI_WSTRB]   = 0;
      intf[eAXI_WLAST]   = 0;
      intf[eAXI_WVALID]  = 0;		
      //
      intf[eAXI_BVALID] = false;
      intf[eAXI_RVALID] = false;

      if (intf_ama[eAMA_WR_REQ]) {
        wr_addr       = intf_ama[eAMA_WR_START_ADDR];
        wr_blen       = intf_ama[eAMA_WR_BURST_LEN];
        wr_start_blen = intf_ama[eAMA_WR_BURST_LEN];
        wr_size       = intf_ama[eAMA_WR_SIZE]; 
        wr_id         = intf_ama[eAMA_WR_ID];

      //fprintf(gFp, "[%s] MAS: WRITE: mid=%s, addr=%s, burst_len=%s, start_blen=%s, size=%s\n", SIM_TM_STR,
      //                               HEX_STR(wr_id), HEX_STR(wr_addr), HEX_STR(wr_blen),
      //                               HEX_STR(wr_start_blen), HEX_STR(wr_size));
        // next move to addr phase
        mas_state = eMAS_WR_ADDR;

      //if (wr_id==2 || wr_id==7)
      //  intf[eAXI_CSYSREQ] = true; // FIX: Add power signals
      }
      else if (intf_ama[eAMA_RD_REQ]) {
        // latch the inputs for rd transaction from app
        rd_addr = intf_ama[eAMA_RD_START_ADDR];
        rd_blen = intf_ama[eAMA_RD_BURST_LEN];
        rd_size = intf_ama[eAMA_RD_SIZE];
        rd_id   = intf_ama[eAMA_RD_ID];

      //fprintf(gFp, "[%s] MAS: READ: mid=%s, addr=%s, burst=%s, size=%s\n", SIM_TM_STR,
      //                               HEX_STR(rd_id), HEX_STR(rd_addr), HEX_STR(rd_blen),
      //                               HEX_STR(rd_size));

        mas_state = eMAS_RD_ADDR;

      //if (rd_id==5 || rd_id==4)
      //  intf[eAXI_CSYSREQ] = true;
      }
    break;
    //
    case eMAS_WR_ADDR:
      intf[eAXI_AWADDR]  = wr_addr;
      intf[eAXI_AWLEN]   = wr_blen;
      intf[eAXI_AWBURST] = 1; // INCR;
      intf[eAXI_AWSIZE]  = wr_size;
      intf[eAXI_AWID]    = wr_id;
      intf[eAXI_AWVALID] = true;

      if (intf[eAXI_AWREADY]) {
        // request to the app to start sending write data
        intf_ama[eAMA_WR_DATA_REQ] = true;
        // move to next phase, data.
        // sample our own data req signal to give 2 clocks
        // so data has enough time to be driven, then sampled on first clock
        // in the next state
        if (intf_ama[eAMA_WR_DATA_REQ]) {
          mas_state = eMAS_FIRST_WR_DATA;
          intf_ama[eAMA_WR_DATA_REQ] = false; 
        }
        intf[eAXI_AWVALID] = false;
      }
    break;
    //
    case eMAS_FIRST_WR_DATA:
      intf[eAXI_WVALID] = 1;
      intf[eAXI_WLAST]  = (wr_blen == 0);
      intf[eAXI_WDATA]  = intf_ama[eAMA_WR_DATA]; // FIX: does this work, assigning out port to in port
      intf[eAXI_WID]    = wr_id;
      intf[eAXI_WSTRB]  = 0xf;

      if (intf[eAXI_WREADY]) {
        // all of the data is valid (for now)
        if (wr_blen > (unsigned)0) wr_blen = wr_blen - (unsigned)1;

        intf[eAXI_WVALID] = false;

        if (wr_blen == 0) {
          intf_ama[eAMA_WR_DATA_REQ] = false;
          mas_state                  = eMAS_WR_RESP;
          intf[eAXI_WLAST]           = false;
        }
        else {
          intf_ama[eAMA_WR_DATA_REQ] = true;
          mas_state              = eMAS_WR_DATA;
        }

      //fprintf(gFp, "[%s] MAS: WRITE: id=%s, data=%s\n", SIM_TM_STR,
      //              HEX_STR(wr_id), HEX_STR((wr_data_i)));
      }
    break;
    //
    case eMAS_WR_DATA:
      intf[eAXI_WVALID]          = intf_ama[eAMA_WR_DATA_VALID];
      intf_ama[eAMA_WR_DATA_REQ] = intf[eAXI_WREADY]; // middle of burst, ready from slave = data req

      // drive write data directly fropm app when valid
      if (intf_ama[eAMA_WR_DATA_VALID]) {  
        intf[eAXI_WDATA] = intf_ama[eAMA_WR_DATA];
        intf[eAXI_WID]   = intf_ama[eAMA_WR_ID];
        // all of the data is valid (for now)
        intf[eAXI_WSTRB] = 0xf;
        if (wr_blen > 0)
          wr_blen = wr_blen - (unsigned)1;

      //fprintf(gFp, "[%s] MAS: WRITE: id=%s, data=%s\n", SIM_TM_STR,
      //              HEX_STR(wr_id), HEX_STR((wr_data_i)));
      }
  
      // drive last signal
      if (wr_blen == 0) {
        intf[eAXI_WLAST]           = true;
        intf_ama[eAMA_WR_DATA_REQ] = false; // about to end burst, de-assert request
        mas_state                  = eMAS_WR_RESP;
      }
    break;
    //
    case eMAS_WR_RESP:
      intf[eAXI_WID]    = 0;
      intf[eAXI_WLAST]  = 0;
      intf[eAXI_WVALID] = 0;
       
      // if response valid is asserted, capture slave response
      if (intf[eAXI_BVALID]) {
        intf[eAXI_BVALID] = true;
        intf[eAXI_BREADY] = true;

        intf_ama[eAMA_WR_ID]  = intf[eAXI_BID];

        if (intf[eAXI_BRESP] == (unsigned)eOKAY) {
          mas_state = eMAS_IDLE;
        }
        else if (intf[eAXI_BRESP] != eOKAY) {
          intf_ama[eAMA_WR_ERR] = true;

        //fprintf(gFp, "[%s] MAS: WRITE: ERROR, got slave resp=%s\n", SIM_TM_STR,
        //              HEX_STR((bresp_i)));
        }

      //fprintf(gFp, "[%s] MAS: WRITE: Completed, resp=%s\n", SIM_TM_STR,
      //              HEX_STR((bresp_i)));
      }
    break;
    //
    case eMAS_RD_ADDR:
      // drive addr phase parameters, assert valid
      intf[eAXI_ARADDR]    = rd_addr;
      intf[eAXI_ARLEN]     = rd_blen;
      intf[eAXI_ARBURST]   = 1; // always INCR, for now
      intf[eAXI_ARSIZE]    = rd_size;
      intf[eAXI_ARID]      = rd_id;
      intf[eAXI_ARVALID]   = true;

      if (intf[eAXI_ARREADY]) {
        // move to next phase, data.
        mas_state          = eMAS_RD_DATA;
        intf[eAXI_ARVALID] = false;

      //fprintf(gFp, "[%s] MAS: READ: mid=%s, addr=%s, burst_len=%s, INCR, size=%s\n", SIM_TM_STR,
      //              HEX_STR(rd_id), HEX_STR(rd_addr), HEX_STR(rd_blen), HEX_STR(rd_size));
      }
    break;
    //
    case eMAS_RD_DATA:
      // handle read data coming back from the slave
      // make sure incoming read ID matches one we are on
      // currently
      if (intf_ama[eAMA_RD_ID] == rd_id) {
        // read valid means data is valid, and 
        // rready is always assumed to be 1
        intf[eAXI_RREADY] = true;
        if (intf[eAXI_RVALID]) {  
          // drive write data coming in from app
          intf_ama[eAMA_RD_DATA]       = intf[eAXI_RDATA];
          intf_ama[eAMA_RD_DATA_VALID] = true;

          // drive the read data ID back to the app
          intf_ama[eAMA_RD_ID] = intf[eAXI_RID];

        //fprintf(gFp, "[%s] MAS: READ: rid=%s, data=%s\n", SIM_TM_STR,
        //              HEX_STR((rid_i)), HEX_STR((rdata_i)));

          // if this is the lastdata phase, process slave response
          // assuming slave response is in-line with rlast
          if (intf[eAXI_RLAST]) {
            intf[eAXI_RVALID]            = true;
            intf_ama[eAMA_RD_DATA_VALID] = false;

            if (intf[eAXI_RRESP] == (unsigned)eOKAY) intf[eAXI_RREADY] = false;
            else                                     intf_ama[eAMA_RD_ERR] = true; // check: what about rready_o in this case?

            mas_state = eMAS_IDLE;

          //fprintf(gFp, "[%s] MAS: READ: Completed, resp=%s", SIM_TM_STR,
          //              HEX_STR((rresp_i)));
          }
        // de-assert read data valid back to app when slave
        // deasserts it's valid
        } else  
          intf_ama[eAMA_RD_DATA_VALID] = false;
      }
    break;
    }
  }
}

// End
