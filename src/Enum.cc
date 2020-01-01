// $RCSfile: Enum.cc,v $
// $Revision: 1.6 $

#include <assert.h>
#include <string.h>

#include "Enum.h"
//
#include "Global.h"
#include "Value.h"
#include "Process_RW.h"

NameVal TypesNV[] =
{
  eType_Aggregate,              "Aggregate",
  eType_Attribute,              "Attribute",
  eType_Bit,                    "Bit",
  eType_Bool,                   "Bool",
  eType_Bucket,                 "Bucket",
  eType_BucketList,             "BucketList",
  eType_Char,                   "Char",
  eType_Clock,                  "Clock",
  eType_Condition,              "Condition",
  eType_DBTransaction,          "DBTransaction",
  eType_DBTransactionRec,       "DBTransactionRec",
  eType_DBTransactionRecValue,  "DBTransactionRecValue",
  eType_Database,               "Database",
  eType_DecodeEncode8b10b,      "DecodeEncode8b10b",
  eType_Event,                  "Event",
  eType_EventFromTrigger,       "EventFromTrigger",
  eType_Interface,              "Interface",
  eType_Link,                   "Link",
  eType_List,                   "List",
  eType_ListIterator,           "ListIterator",
  eType_MailboxBase,            "MailboxBase",
  eType_Mailbox,                "Mailbox",
  eType_Module,                 "Module",
  eType_ModuleTop,              "ModuleTop",
  eType_Module_PhyUsbPcie,      "Module_PhyUsbPcie",
  eType_Mutex,                  "Mutex",
  eType_PRNG,                   "PRNG",
  eType_Pointer,                "Pointer",
  eType_Process,                "Process",
  eType_ProcessAndEdgeInfo,     "ProcessAndEdgeInfo",
  eType_ProcessExecuteReason,   "ProcessExecuteReason",
  eType_Process_PipeSerial,     "Process_PipeSerial",
  eType_Process_RW,             "Process_RW",
  eType_Query,                  "Query",
  eType_Record,                 "Record",
  eType_Scheduler,              "Scheduler",
  eType_Semaphore,              "Semaphore",
  eType_SemaphoreInfo,          "SemaphoreInfo", 
  eType_SensitiveInfo,          "SensitiveInfo",
  eType_Short,                  "Short",
  eType_Signal,                 "Signal",
  eType_Simulator,              "Simulator",
  eType_SimulatorTime,          "SimulatorTime",
  eType_Stack,                  "Stack",
  eType_String,                 "String",
  eType_Table,                  "Table",
  eType_TableRecord,            "TableRecord",
  eType_Timer,                  "Timer",
  eType_Trigger,                "Trigger",
  eType_Unsigned,               "Unsigned",
  eType_Value,                  "Value",
  eType_Vcd,                    "Vcd",
  eType_View,                   "View",
  eType_ViewIterator,           "ViewIterator",
  eType_Max,                    "Max"
};

bool EnumExists(NameVal nv[], short code)
{
  short i;

  for (i=0; nv[i].val != eEnumInvalid; i++)
  {
    if (nv[i].val == code)
      return TRUE;
  }

  return FALSE;
}

const char* Enum2Str(NameVal nv[], short code)
{
  short i;

  for (i=0; nv[i].val != eEnumInvalid; i++)
  {
    if (nv[i].val == code)
      return nv[i].name;
  }

  printf("Error: Did not find string for enum %d for NameVals starting with %s\n", code, nv[0].name);

  return "Error:Enum2Str_NotFound";
}

short Str2Enum(NameVal nv[], char* name)
{
  short i;

  for (i=0; nv[i].val != eEnumInvalid; i++)
  {
    if (!strcmp(nv[i].name, name))
      return nv[i].val;
  }

  return eEnumInvalid;
}

// End
