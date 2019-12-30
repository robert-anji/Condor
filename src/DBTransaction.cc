// $RCSfile: DBTransaction.cc,v $
// $Revision: 1.1 $

#include "DBTransaction.h"

DBTransaction::DBTransaction(eDBTransactionOperation oper) :
  mTransOper(oper)
{
}

DBTransaction::~DBTransaction()
{
}

// End
