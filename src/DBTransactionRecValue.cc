// $RCSfile: DBTransactionRecValue.cc,v $
// $Revision: 1.3 $

#include <assert.h>

#include "DBTransactionRecValue.h"
//
#include "Global.h"
#include "Value.h"

M_ExternConstructDestructCountInt;

DBTransactionRecValue::DBTransactionRecValue(eDBTransactionOperation tran_oper, View *view, Record *rec, const Attribute* attr, const Value &val) :
  DBTransaction   (tran_oper),
  mpView          (view),
  mpRecord        (rec),
  mpAttribute     (attr)
{
  mpValue = new Value(val); M_Constructed(Value);

  assert (mpValue);

  M_UpdConstructCount(eType_DBTransactionRecValue);
}

DBTransactionRecValue::~DBTransactionRecValue()
{
  if (mpValue) { delete mpValue; mpValue = 0; }

  M_UpdDestructCount(eType_DBTransactionRecValue);
}

// End
