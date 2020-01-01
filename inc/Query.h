// $RCSfile: Query.h,v $
// $Revision: 1.9 $

#ifndef QueryClass
#define QueryClass

#include "BucketList.h"
#include "Condition.h"
#include "Record.h"

class Query
{
  friend class ViewJoin;

public:
  Query();
  Query(const char* name);
  Query(Query &q);
  //
 ~Query();

  bool Match(Record* rec=0);

  Query& operator =(Condition& c);
  Query& operator&=(Condition& c); // TBD
  Query& operator|=(Condition& c); // TBD
  // Note: Query also is responsible for the deletion of
  //       dynamically allocated Condition objects.

  Query& operator&&(Query& q);
  Query& operator&=(Query& q);
  Query& operator||(Query& q);
  Query& operator|=(Query& q);

  bool QueryEdit(short condition_id, Value& val);
  bool QueryEdit(short condition_id, unsigned u);
  bool QueryModifyConditionFirst(Value* val, eCondOperatorType typ=eCondOper_Invalid);

  void Print();
  void p() { Print(); }

private:
  BucketList<Condition> mConditionBktList;

  const char*    mpName; // name is needed when queries have to be persistent
};

#endif

// End
