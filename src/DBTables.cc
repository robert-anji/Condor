// $RCSfile: DBTables.cc,v $
// $Revision: 1.3 $

#include "DBTables.h"

// Define Table attributes here. Tables are:
// Synchronous_Process_table or simply called Process_table for short
// Asynchronous_Process_table

/////////// Process_table
const char* cAttr_Processes_SimTime       = "SimTime";
const char* cAttr_Processes_DeltaCycle    = "DeltaCycle";
const char* cAttr_Processes_Region        = "Region";
const char* cAttr_Processes_Name          = "ProcessName";
const char* cAttr_Processes_Clock         = "Clock";
const char* cAttr_Processes_ClockEdge     = "ClockEdge";
const char* cAttr_Processes_ProcessPtr    = "ProcessPtr";
const char* cAttr_Processes_ExecReasonPtr = "ExecReasonPtr";
const char* cAttr_Processes_InterfacePtr  = "InterfacePtr";
const char* cAttr_Processes_BlockingAgent = "BlockingAgent";
const char* cAttr_Processes_BlockingAgentArg = "BlockingAgentArg";

TableAttributes cProcessesTableAttr [] =
{
  cAttr_Processes_SimTime,      eValueUnsigned,
  cAttr_Processes_DeltaCycle,   eValueUnsigned,
  cAttr_Processes_Region,       eValueUnsigned,
  cAttr_Processes_Name,         eValueCharPtr,
  cAttr_Processes_Clock,        eValueVoidPtr,
  cAttr_Processes_ClockEdge,    eValueUnsigned,
  cAttr_Processes_ProcessPtr,   eValueVoidPtr,
  cAttr_Processes_ExecReasonPtr,eValueVoidPtr,
  cAttr_Processes_InterfacePtr, eValueVoidPtr,
  cAttr_Processes_BlockingAgent,eValueVoidPtr,
  cAttr_Processes_BlockingAgentArg,eValueUnsigned,
  "",                           eValueUnknown
};

/////////// Asynchronous_Process_table
const char* cAttr_AsyncProcesses_ProcessPtr    = "ProcessPtr";
const char* cAttr_AsyncProcesses_InterfacePtr  = "InterfacePtr";

TableAttributes cAsyncProcessesTableAttr [] =
{
  cAttr_AsyncProcesses_ProcessPtr,   eValueVoidPtr,
  cAttr_AsyncProcesses_InterfacePtr, eValueVoidPtr,
  "",                                eValueUnknown
};

// End
