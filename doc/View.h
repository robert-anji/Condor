// $RCSfile: View.h,v $
// $Revision: 1.2 $
// 1. Prefixed record functions with 'Tran'.
// 2. Removed ValueSeparatorGet().
// 3. Made RecordSearchXXX() functions return the record pointer.

#ifndef ViewClass
#define ViewClass

#include <assert.h>

#include "Global.h"
#include "Database.h"
#include "DefinesAndEnum.h"
#include "Scheduler.h"
#include "List.h"

class Attribute;
class Aggregate;
class Process;
class Query;
class Record;
class Table;
class Trigger;
class Value;
class ViewIterator;

//\class View
/// Some description
///

//\note {
// This is one of the most import classes to use the database
// For VerifyRight!
//}

class View
{
public:
  friend class Database;
  friend class Table;

  // Constructor/Desctructor are private, used only by friend class: Table.

  // Records API (iterate, search, retrieve, delete records etc.):
  //   select  records (also allows this to be accomplished iteratively)
  //   import  records (the records that were selected)
  //   import  records (dynamically, using triggers)
  //   own the records (set ownership/permission)
  //   release records (decrements the reference count)
  //   delete  records (delete will occur when reference count becomes 0)

  bool     ViewRecordUnselect     (Record *rec);
  unsigned TableRecordsSelect     (unsigned start, unsigned end);
  unsigned TableRecordsSelect     (Query* q);
  // \
  unsigned TableRecordsSelect     (Regex* r); // TBD: upcoming feature
  //
  bool     TableRecordsImportSelected();
  bool     TableRecordsImportFromTrigger(Trigger *trigger);
  //
  Record*  RecordNew           ();
  // The functions below are overloaded to accept an optional iterator.
  Record*  RecordCurrentGet    (            ViewIterator* iter=0);
  Record*  RecordSearchFirst   (Query* q=0, ViewIterator* iter=0);

  unsigned Size() { return mpRecordPtrList->Size(); }

  // Trigger API:
  ///////////////
  // Trigger for records.
  Trigger* TriggerNew(eTriggerType type, void *trg_owner_obj, Query *q=0, Record *rec=0);
  // Trigger for aggregates.
  Trigger* TriggerNew(eTriggerType type, void *trg_owner_obj, Aggregate *aggr, Query *q);
  // Trigger activate action: invoke callback TriggerActionFnPtr or block 'process'.
  bool TriggerActivate(Trigger* trg, TriggerActionFnPtr fn);
  bool TriggerActivate(Trigger* trg, Process* process);
  // Trigger management.
  bool     TriggerSuspend   (Trigger *trg);
  bool     TriggerActivate  (Trigger *trg);
  bool     TriggerDelete    (Trigger *trg);
  bool     TriggerMatched   (Trigger *trg);

  List<Record>*          mpRecordPtrList;   // all records selected by view

private:
  const char*            mpName;
  Table*                 mpTable;  // the table from which the view was made
  eRecordPermissionBits  mRecPermission;
//List<Record>*          mpRecordPtrList;   // all records selected by view
  List<Aggregate>        mAggregatePtrList; // all aggregates added to view
  List<Record>           mRecordNewPtrList; // all new records, not added yet to table
  List<ViewIterator>     mViewIteratorList;

  // Private member functions:
  View  (const char* view_name, Table* table, eRecordPermissionBits perm);
  ~View ();
};

#endif

// End
