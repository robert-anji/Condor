// $RCSfile: DBTransaction.h,v $
// $Revision: 1.2 $

#ifndef DBTransactionClass
#define DBTransactionClass

// Base class for all Database Transactions.

#include "Database.h"

class DBTransaction
{
public:
  DBTransaction(eDBTransactionOperation oper);
  virtual ~DBTransaction();

  eDBTransactionOperation  mTransOper;
private:
};

#endif

// End
