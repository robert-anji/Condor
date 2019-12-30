// $RCSfile: DBTables.h,v $
// $Revision: 1.4 $

#ifndef DBTables
#define DBTables

#include "DBDefines.h"

struct StructTableAttributes {
  const char* mAttributeName;
  eValueType  mAttributeType;
};
typedef StructTableAttributes TableAttributes;

// Process_table
extern const char* const cProcessName;
extern const char* const cProcessPtr;
extern const char* const cProcessState;
extern const char* const cProcessClockName;
extern const char* const cProcessClockEdge;

// Synchronous_Process_table
extern const char* cAttr_Processes_SimTime;
extern const char* cAttr_Processes_DeltaCycle;
extern const char* cAttr_Processes_Region;
extern const char* cAttr_Processes_Name;
extern const char* cAttr_Processes_Clock;
extern const char* cAttr_Processes_ClockEdge;
extern const char* cAttr_Processes_ProcessPtr;
extern const char* cAttr_Processes_ExecReasonPtr;
extern const char* cAttr_Processes_InterfacePtr;
extern const char* cAttr_Processes_BlockingAgent;
extern const char* cAttr_Processes_BlockingAgentArg;

// Asynchronous_Process_table
extern const char* cAttr_AsyncProcesses_ProcessPtr;
extern const char* cAttr_AsyncProcesses_InterfacePtr;

#endif

// End
