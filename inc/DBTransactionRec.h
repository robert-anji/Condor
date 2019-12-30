// $RCSfile: DBTransactionRec.h,v $
// $Revision: 1.4 $

#ifndef DBTransactionRecClass
#define DBTransactionRecClass

#include "Database.h"
#include "DBTransaction.h"

class Record;
class Table;
class View;

class DBTransactionRec : public DBTransaction
{
  friend class Database;

public:
  DBTransactionRec(eDBTransactionOperation tran_oper, View *view, Record *rec);
  virtual ~DBTransactionRec();

private:
  Record    *mpRecord;
  View      *mpView;
};

#endif

// End
