// $RCSfile: Trigger.cc,v $
// $Revision: 1.6 $

#include <assert.h>

#include "Trigger.h"
//
#include "Condition.h"
#include "Global.h"
#include "Query.h"
#include "Record.h"

M_ExternConstructDestructCountInt;
// TriggerMatchAggregate;

Trigger::Trigger(const char* trg_name, eTriggerType type, void *trg_owner_obj, TriggerActionFunctionPtr fn_ptr, View *v, Query *q, Record *rec) :
  mName                (trg_name),
  mTriggerType         (type),
  mpTriggerOwnerObj    (trg_owner_obj),
  mTriggerActionFnPtr  (fn_ptr),
  mpView               (v),
  mpQuery              (0),
  mpRecord             (rec),
  mTriggerState        (eTriggerState_Active),
  mMatchedCount        (0)
{
  if (q) { mpQuery = new Query(*q); assert(mpQuery); }

  M_UpdConstructCount(eType_Trigger);
}

Trigger::~Trigger()
{
  if (mpQuery) { delete mpQuery; mpQuery=0; }

  M_UpdDestructCount(eType_Trigger);
}

void Trigger::TriggerActionExecute()
{
  if (mTriggerActionFnPtr) (*mTriggerActionFnPtr)(this);
  else                     mMatchedCount++;
}

bool Trigger::TriggerMatch(Record *rec)
{
  if (rec) {
    if      (mpRecord) if (mpRecord != rec) return FALSE;
                       else                 return mpQuery->Match(rec);
    else if (mpQuery)  return mpQuery->Match(rec);
    else               return TRUE;
  }

  // The remaining logic is for rec==0

  // rec is null but Trigger needs to match mpRecord, then return FALSE.
  if (mpRecord) return FALSE;
  
  assert(mpQuery); // cannot have rec==0, mpRecord==0 and mpQuery==0

  return mpQuery->Match();
}

bool Trigger::TriggerStateSet(eTriggerState trg_state)
{
  mTriggerState = trg_state;

  return TRUE;
}

eTriggerType Trigger::TriggerType() const
{
  return mTriggerType;
}

eTriggerState Trigger::TriggerStateGet() const
{
  return mTriggerState;
}

void Trigger::Clear()
{
  mMatchedCount = 0;
}

// End
