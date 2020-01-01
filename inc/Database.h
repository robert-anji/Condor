// $RCSfile: Database.h,v $
// $Revision: 1.7 $

#ifndef DatabaseClass
#define DatabaseClass

#include "Global.h"
#include "DBDefines.h"
#include "List.h"

class Attribute;
class DBTransaction;
class Query;
class Record;
class Scheduler;
class Table;
class Trigger;
class Value;
class View;

typedef void (*TriggerActionFunctionPtr)(Trigger *trigger);
typedef        TriggerActionFunctionPtr  TriggerActionFnPtr;

const TriggerActionFnPtr TriggerActionFnPtrNone = (TriggerActionFnPtr)eType_Max;

enum eDBTransactionOperation {
  eDBTransRecOper_RecordAdd,
  eDBTransRecOper_RecordDelete,
  eDBTransRecOper_RecordValueModify,
  eDBTransRecOper_TableDelete,
  eDBTransRecOper_Invalid,
};

class Database
{
  friend class Scheduler;

public:
  // Constructor/Destructor:
  Database(const char* path_to_db, const char* db_name);
  ~Database();

  // Database Table API:
  Table*   TableOpen (const char* table_name, int flags);
  bool     TableClose(Table* tbl);

  // Database Transactions API:
  //   Note: We add prefix DB to differentiate it from hardware 'transactions'.
  //   Note: Currently, only Table class expected to call these functions.
  void     DBTransactionsAdd(eDBTransactionOperation oper, View *view, Record *rec);
  void     DBTransactionsAdd(eDBTransactionOperation oper, View *view, Record *rec, const Attribute* attr, const Value &v);
  short    DBTransactionsCommitView(View* view);

private:
  // Data:
  const char*          mpPathToDB;
  const char*          mpName;
  //
  List<Table>          mTableList;
  List<DBTransaction>  mDBTransactionList;
  List<Trigger>        mTriggerList;

  // Member functions.
  short    DBTransactionsCommitAll() { return DBTransactionsCommitView(0); }
};

#endif

// End
