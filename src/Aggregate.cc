// $RCSfile: Aggregate.cc,v $
// $Revision: 1.6 $

#include <assert.h>

#include "Aggregate.h"
//
#include "Attribute.h"
#include "Condition.h"
#include "DBDefines.h"
#include "Value.h"
#include "View.h"

M_ExternConstructDestructCountInt;

Aggregate::Aggregate(View *view, eAggregateType aggr_type, const Attribute* attr) :
  mpAttribute         (attr),
  mAggregateType      (aggr_type),
  mpValue             (0),
  mpValueRemainder    (0),
  mUpdateCount        (0),
  mpView              (view)
{
  mpValue = new Value((unsigned)0); M_Constructed(Value);

  if (mAggregateType == eAggr_ValueAverage) {
    mpValueRemainder = new Value((unsigned)0); M_Constructed(Value);
  }

  M_UpdConstructCount(eType_Aggregate);
}

Aggregate::~Aggregate()
{
  if (mpValue) { delete mpValue; mpValue = 0; }

  if (mpValueRemainder) {
    delete mpValueRemainder; mpValueRemainder = 0;
  }

  M_UpdDestructCount(eType_Aggregate);
}

Value* Aggregate::ValueGet() const
{
  return mpValue;
}

Value* Aggregate::ValueRemainderGet() const
{
  return mpValueRemainder;
}

void Aggregate::ValueReset()
{
  mpValue          = (unsigned)0;
  mpValueRemainder = (unsigned)0;
}

void Aggregate::ValueUpdate(eAggregateUpdateReasonType rsn_type, Value *changed_val, Value *old_val)
{
  // TBD: Support negative aggregates.
  unsigned changed_value = (unsigned)(*changed_val);
  unsigned old_value     = (unsigned)(*old_val);
  unsigned value         = (unsigned)(*mpValue);
  unsigned value_rem     = (unsigned)(*mpValueRemainder);

  switch (mAggregateType) {
    case eAggr_None:
      break;
    case eAggr_ValueAverage:
      switch (rsn_type) {
        case eAggrUpdType_RecordAdd:
          assert (mUpdateCount != UINT_MAX);

          mUpdateCount++;

          value     = (value*(mUpdateCount-1) + changed_value + value_rem)/mUpdateCount;
          value_rem = (value*(mUpdateCount-1) + changed_value + value_rem)%mUpdateCount;
          break;
        case eAggrUpdType_RecordDelete:
          assert(mUpdateCount);

          // Sticky implies not to update on 'Delete'.
          if (mAggrIsSticky) break;

          mUpdateCount--;

          if (mUpdateCount) {
            value     = (value*(mUpdateCount+1) - changed_value + value_rem)/mUpdateCount;
            value_rem = (value*(mUpdateCount+1) - changed_value + value_rem)%mUpdateCount;
          }
          else {
            value     = 0;
            value_rem = 0;
          }
          break;
        case eAggrUpdType_RecordModify:
          value     = (value*(mUpdateCount+1) + changed_value - old_value + value_rem)/mUpdateCount;
          value_rem = (value*(mUpdateCount+1) + changed_value - old_value + value_rem)%mUpdateCount;
          break;
        default:
          assert(0);
      }
      *mpValue          = value;
      *mpValueRemainder = value_rem;
      break;
    case eAggr_ValueMinimum:
      switch (rsn_type) {
        case eAggrUpdType_RecordAdd:
          if (*changed_val < *mpValue) *mpValue = *changed_val;
          break;
        case eAggrUpdType_RecordDelete:
        case eAggrUpdType_RecordModify:
          if (*changed_val <  *mpValue) assert(0);
          if (*changed_val == *mpValue) mpView->ValueMin(mpAttribute, mpValue);
          break;
        default:
          assert(0);
      }
      break;
    case eAggr_ValueMaximum:
      switch (rsn_type) {
        case eAggrUpdType_RecordAdd:
          if (changed_val > mpValue) mpValue = changed_val;
          break;
        case eAggrUpdType_RecordDelete:
        case eAggrUpdType_RecordModify:
          if (*changed_val >  *mpValue) assert(0);
          if (*changed_val == *mpValue) mpView->ValueMax(mpAttribute, mpValue);
          break;
        default:
          assert(0);
      }
      break;
    case eAggr_ValueSum:
      switch (rsn_type) {
        case eAggrUpdType_RecordAdd:
          *mpValue += *changed_val;
          break;
        case eAggrUpdType_RecordDelete:
          *mpValue -= *changed_val;
          break;
        case eAggrUpdType_RecordModify:
          *mpValue += *changed_val;
          *mpValue -= *old_val;
          break;
        default:
          assert(0);
      }
      break;
    case eAggr_RecordsCount:
      switch (rsn_type) {
        case eAggrUpdType_RecordAdd:    *mpValue++; break;
        case eAggrUpdType_RecordDelete: *mpValue--; break;
      }
      break;
    case eAggr_RecordsAdded:
      switch (rsn_type) {
        case eAggrUpdType_RecordAdd:    *mpValue++; break;
      }
      break;
    case eAggr_RecordsDeleted:
      assert(*mpValue > 0); // for record delete value should not be 0

      switch (rsn_type) {
        case eAggrUpdType_RecordDelete: *mpValue--; break;
      }
      break;
    default:
      assert(0);
  }
}

Condition& Aggregate::operator<(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c < val;
}

Condition& Aggregate::operator>(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c > val;
}

Condition& Aggregate::operator<=(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <= val;
}

Condition& Aggregate::operator>=(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >= val;
}

Condition& Aggregate::operator==(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == val;
}

Condition& Aggregate::operator!=(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != val;
}

// End
