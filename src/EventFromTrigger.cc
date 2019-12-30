// $RCSfile: EventFromTrigger.cc,v $
// $Revision: 1.2 $

#include "EventFromTrigger.h"

EventFromTrigger::EventFromTrigger(const char* ev_name, eTriggerType type, View *v, Query *q, Record *rec) :
  Trigger(ev_name, type, (void*)0, TriggerActionFnPtrNone, v, q, rec),
  Event(ev_name)
{
  M_UpdConstructCount(eType_EventFromTrigger);
}

EventFromTrigger::~EventFromTrigger()
{
  M_UpdDestructCount(eType_EventFromTrigger);
}

void EventFromTrigger::TriggerActionExecute()
{
  Set();
}

// End
