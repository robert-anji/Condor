// $RCSfile: Aggregate.h,v $
// $Revision: 1.6 $

#ifndef AggregateClass
#define AggregateClass

#include "Condition.h"
#include "Global.h"
#include "DBDefines.h"

class Attribute;
class Value;
class View;

class Aggregate
{
  friend Value* Condition::ValuePtrGet(const Aggregate* aggr) const;

public:
  Aggregate(View* view, eAggregateType type, const Attribute* attr);
  ~Aggregate();

  // Overloaded operators needed to convert Aggregates into queries on-the-fly.
  #include "ConditionCreateOperators.h"

  Value*   ValueGet()           const;
  Value*   ValueRemainderGet()  const;
  void     ValueReset();
  void     ValueUpdate(eAggregateUpdateReasonType type, Value *changed_v, Value *old_v);

  void     MakeSticky(bool sticky=TRUE) { mAggrIsSticky = sticky; }

  const Attribute* AttributeGet() const { return mpAttribute; }

private:
  const Attribute*    mpAttribute;
  eAggregateType      mAggregateType;
  bool                mAggrIsSticky;
  Value*              mpValue;
  Value*              mpValueRemainder;// we use 'remainder' to avoid float
  unsigned            mUpdateCount;    // number of values aggregate'd
  View*               mpView;          // sometimes need access to view's recs.

  // TBD: (next 2 lines) \
  Query              *mpQuery; \
  Regex              *mpRegex;
};

#endif

// End
