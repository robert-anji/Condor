// $RCSfile: ModuleTop.cc,v $
// $Revision: 1.3 $

#include "ModuleTop.h"
//
#include "Clock.h"
#include "Process_RW.h"
#include "SimulatorDefines.h"

#include "InterfaceDesc_RW.h"
#include "InterfaceDesc_Clock.h"
#include "InterfaceDesc_AXI.h"

M_ExternConstructDestructCountInt;

Interface* intf_axi_mas;
Interface* intf_axi_slv;

ModuleTop::ModuleTop(const char* name) :
  Module(name, 0)
{
  Clock* clk4 = new Clock("Clock1", this, 4, eTimeScale_ns, eClockPhase_JustAfterNegativeEdge, InterfaceDesc_Clock);
  Clock* clk6 = new Clock("Clock2", this, 6, eTimeScale_ns, eClockPhase_JustAfterNegativeEdge, InterfaceDesc_Clock);

  Interface* intf_mas = new Interface("Interface_RW_Mas", InterfaceDesc_RW);
  Interface* intf_slv = new Interface("Interface_RW_Slv", InterfaceDesc_RW, eIntfModifier_SignalDirInvert);

  intf_axi_mas = new Interface("AXI_MAS", InterfaceDesc_AXI);
  intf_axi_slv = new Interface("AXI_SLV", InterfaceDesc_AXI, eIntfModifier_SignalDirInvert);

  ConnectInterfaceGroup(intf_mas, intf_slv,             "RW",  eInterfaceConnectType_Peer);
  ConnectInterfaceGroup(intf_mas, clk4->InterfaceGet(), "CLK", eInterfaceConnectType_Peer);
  ConnectInterfaceGroup(intf_slv, clk4->InterfaceGet(), "CLK", eInterfaceConnectType_Peer);

  Process* master_proc = new Process_RW("Master", this, Process_RW::eRW_Master);
  Process* slave_proc  = new Process_RW("Slave",  this, Process_RW::eRW_Slave);

  master_proc->InterfaceSet(intf_mas);
  slave_proc->InterfaceSet(intf_slv);

  // Note: it is ok to mark signals as sensitive before/after connecting them.
  master_proc->ProcessIsSensitiveTo(cRWIntf_Clk, eSensitiveType_PositiveEdge);
  slave_proc->ProcessIsSensitiveTo(cRWIntf_Clk,  eSensitiveType_PositiveEdge);

  // Q. How to assert reset pin if present, need another process for this?
  // Maybe add reset pin for the RW interface, syntax will be:
  // intf_mas[cRWIntf_Reset] = 0;
  // Or, can we construct a PinReset object?
  // PinReset pin_reset(start_tm, end_tm, reset_level);
  // ConnectInterfaceGroup(intf_mas, pin_reset->InterfaceGet(), "RST");

  M_UpdConstructCount(eType_ModuleTop);
}

ModuleTop::~ModuleTop()
{
  // Allocated processes are stored and deleted in the Module base class.
  delete intf_axi_mas;
  delete intf_axi_slv;

  M_UpdDestructCount(eType_ModuleTop);
}

// End
