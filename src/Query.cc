// $RCSfile: Query.cc,v $
// $Revision: 1.5 $

#include "Query.h"
//
#include "Condition.h"
#include "Global.h"
#include "Value.h"

M_ExternConstructDestructCountInt;

Query::Query() :
  mpName("NoName")
{
  M_UpdConstructCount(eType_Query);
}

Query::Query(const char* name) :
  mpName(name)
{
  M_UpdConstructCount(eType_Query);
}

Query::Query(Query &q) :
  mpName("NoName")
{
  mConditionBktList.CopyDeep(q.mConditionBktList);

  M_UpdConstructCount(eType_Query);
}

Query::~Query()
{
  for (MacroBucketList_BucketIterateAll(mConditionBktList)) {
    for (MacroBucketList_ListIterateAll(mConditionBktList)) {
      Condition* c = mConditionBktList.Get();

      if (c) { delete c;  }
      else   { assert(0); }
    }
  }

  M_UpdDestructCount(eType_Query);
}

bool Query::Match(Record* rec)
{
  // This Match handles non-join queries. All conditions within the query are
  // expected to contain just one Attribute which will be for the LHS. The RHS
  // will be a value or value pointer.
  //
  // Note: This function is also used for rec=0

  BucketList<Condition> & cbl = mConditionBktList;

  if (!cbl.BktSetPosition(ePositionFirst)) {
    // If query is empty then there are no conditions
    // and the record is defined to always match.
    return TRUE;
  }

  // We have a match IF the conditions within ANY bucket are ALL true
  for (cbl.BktSetPosition(ePositionFirst); !cbl.BktDone(); cbl.BktSetPosition(ePositionNext))
  {
    // Process the 'and' condition, break as soon as there is no match.
    for (cbl.SetLinkPosition(ePositionFirst); !cbl.LinkDone(); cbl.SetLinkPosition(ePositionNext))
    {
      Condition* c = cbl.Get();

      // TBD: Will check for RecordReference here and find the referred to record.

      if (c->Match(rec))  continue;
      else                break;
    }

    // Check how we broke out of the previous loop.
    // If all conditions in the 'and' term matched then we are done,
    // else, move on to the next bucket to check for 'or' condititions.
    if (cbl.LinkDone()) return TRUE;
  }

  return FALSE;
}

Query& Query::operator=(Condition& c)
{
  // First free any existing entries in mConditionBktList.
  if (mConditionBktList.Size()) {
    for (MacroBucketList_BucketIterateAll(mConditionBktList)) {
      for (MacroBucketList_ListIterateAll(mConditionBktList)) {
        Condition* c = mConditionBktList.Get();

        if (c) { delete c; c = 0; }
        else   { assert(0);       }
      }
    }
  }
  mConditionBktList.Clean();

  // BucketList assignment operator does all the work.
  if (c.mIsAllocatedAuto) mConditionBktList.CopyShallow(c.mConditionBktList);
  else                    mConditionBktList.CopyDeep   (c.mConditionBktList);

  return *this;
}

Query& Query::operator&&(Query& q)
{
  OperationAnd(mConditionBktList, q.mConditionBktList);

  return *this;
}

Query& Query::operator&=(Query& q)
{
  return (*this && q);
}

Query& Query::operator||(Query& q)
{
  OperationOr(mConditionBktList, q.mConditionBktList);

  return *this;
}

Query& Query::operator|=(Query& q)
{
  return (*this || q);
}

void Query::Print()
{
  for (MacroBucketList_BucketIterateAll(mConditionBktList)) {
    for (MacroBucketList_ListIterateAll(mConditionBktList)) {
      Condition* c = mConditionBktList.Get();

      c->Print(stdout);
      printf(" ");
    }
    printf("\n");
  }
}

bool Query::QueryModifyConditionFirst(Value* val, eCondOperatorType oper)
{
  mConditionBktList.BktSetPosition(ePositionFirst);
  mConditionBktList.SetLinkPosition(ePositionFirst);

  Condition* c = mConditionBktList.Get();
  assert(c);

  if (c) {
    c->mValueRHS = *val;

    if (oper != eCondOper_Invalid) c->mCondOperator = oper;
  }

  return (c != 0);
}

bool Query::QueryEdit(short condition_id, Value& val)
{
  BucketList<Condition> & cbl = mConditionBktList; // shortcut

  bool found = FALSE;

  // We have to scan the entire bucket list as multiple
  // conditions can have the value index set.

  for (cbl.BktSetPosition(ePositionFirst); !cbl.BktDone(); cbl.BktSetPosition(ePositionNext))
  {
    // Process all links in the bucket.
    for (cbl.SetLinkPosition(ePositionFirst); !cbl.LinkDone(); cbl.SetLinkPosition(ePositionNext))
    {
      Condition* c = cbl.Get();
      assert(c);

      if (c->mValueIndex == condition_id) {
        found = TRUE;

        // Update the conditions value.
        c->mValueRHS = val;
      }
    }
  }

  return found;
}

bool Query::QueryEdit(short condition_id, unsigned u)
{
  BucketList<Condition> & cbl = mConditionBktList; // shortcut

  bool found = FALSE;

  // We have to scan the entire bucket list as multiple
  // conditions can have the value index set.

  for (cbl.BktSetPosition(ePositionFirst); !cbl.BktDone(); cbl.BktSetPosition(ePositionNext))
  {
    // Process all links in the bucket.
    for (cbl.SetLinkPosition(ePositionFirst); !cbl.LinkDone(); cbl.SetLinkPosition(ePositionNext))
    {
      Condition* c = cbl.Get();
      assert(c);

      if (c->mValueIndex == condition_id) {
        found = TRUE;

        // Update the conditions value.
        c->mValueRHS = u;
      }
    }
  }

  return found;
}

// End
