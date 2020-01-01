// $RCSfile: DBTransactionRec.cc,v $
// $Revision: 1.3 $

#include "DBTransactionRec.h"

#include "Global.h"

M_ExternConstructDestructCountInt;

DBTransactionRec::DBTransactionRec(eDBTransactionOperation tran_oper, View *view, Record *rec) :
  DBTransaction     (tran_oper),
  mpView            (view),
  mpRecord          (rec)
{
  M_UpdConstructCount(eType_DBTransactionRec);
}

DBTransactionRec::~DBTransactionRec()
{
  M_UpdDestructCount(eType_DBTransactionRec);
}

// End
