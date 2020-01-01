// $RCSfile: EventFromTrigger.h,v $
// $Revision: 1.1 $

#ifndef EventFromTriggerClass
#define EventFromTriggerClass

#include "Event.h"
#include "Trigger.h"

class EventFromTrigger : public Event, public Trigger
{
public:
  EventFromTrigger(const char* ev_name, eTriggerType type, View* v, Query* q=0, Record* rec=0);
  //
 ~EventFromTrigger();

  virtual void TriggerActionExecute();

private:
};

#endif

// End
