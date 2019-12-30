// $RCSfile: Trigger.h,v $
// $Revision: 1.7 $

#ifndef TriggerClass
#define TriggerClass

#include "DBDefines.h"
#include "Database.h"
#include "String.h"

class Condition;
class Query;
class Record;
class View;

enum eTriggerState {
  eTriggerState_Active,
  eTriggerState_Suspended,
  eTriggerState_RemoveTrigger // TBD: Remove this enum, use _Suspended instead
};

class Trigger
{
  friend class Table;

public:
  Trigger(const char* trg_name, eTriggerType type, void* trg_owner_obj, TriggerActionFnPtr fn_ptr, View* v, Query* q=0, Record* rec=0);
 ~Trigger();

  virtual void    TriggerActionExecute();

  bool            TriggerStateSet(eTriggerState trg_state);
  eTriggerState   TriggerStateGet()          const;
  eTriggerType    TriggerType()              const;

  bool            TriggerMatch(Record *rec=0);
  void            TriggerRecordSet(Record* rec) { mpRecord = rec; }

  void*           TriggerOwnerObjGet() const { return mpTriggerOwnerObj; }
  View*           ViewGet() const            { return mpView;            }

private:
  // Data:
  String               mName;
  eTriggerType         mTriggerType;
  TriggerActionFnPtr   mTriggerActionFnPtr;
  void*                mpTriggerOwnerObj;
  View*                mpView;
  Query*               mpQuery;
  Record*              mpRecord; // supplied record, or record for trigger match
//Condition*           mpCondition;
  eTriggerState        mTriggerState;   // Active, Suspended
  int                  mMatchedCount;

  // Functions:
  void     Clear();
};

#endif

// End
