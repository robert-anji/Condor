// $RCSfile: DBTransactionRecValue.h,v $
// $Revision: 1.4 $

#ifndef DBTransactionRecValueClass
#define DBTransactionRecValueClass

#include "Database.h"
#include "DBTransaction.h"

class Attribute;
class Table;
class Value;
class View;

class DBTransactionRecValue : public DBTransaction
{
  friend class Database;

public:
  DBTransactionRecValue(eDBTransactionOperation tran_oper, View *view, Record* rec, const Attribute* attr, const Value &val);
  virtual ~DBTransactionRecValue();

private:
  Table*             mpTable;   // mTable actually not needed now, maybe for triggers
  View*              mpView;
  Record*            mpRecord;
  const Attribute*   mpAttribute;
  Value*             mpValue;
};

#endif

// End
