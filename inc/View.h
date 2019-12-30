// $RCSfile: View.h,v $
// $Revision: 1.10 $

#ifndef ViewClass
#define ViewClass

#include <assert.h>

#include "Global.h"
#include "DBDefines.h"
#include "Database.h"
#include "List.h"

M_ExternConstructDestructCountInt;

class Attribute;
class Aggregate;
class Process;
class Query;
class Record;
class Table;
class Trigger;
class Value;
class ViewIterator;

class View
{
  friend class Database;
  friend class Table;

public:
  // Constructor/Desctructor are private, used only by friend class: Table.

  // Records API (iterate, search, retrieve, delete records etc.):
  ////////////////////////////////////////////////////////////////
  //   select  records (also allows this to be accomplished iteratively)
  //   import  records (the records that were selected)
  //   import  records (dynamically, using triggers)
  //   own the records (set ownership/permission)
  //   release records (decrements the reference count)
  //   delete  records (delete will occur when reference count becomes 0)
  //
  unsigned TableRecordsSelect     (Query* q);
  unsigned TableRecordsSelect     (unsigned start, unsigned end);
  bool     ViewRecordUnselect     (Record *rec);
  // \
  unsigned TableRecordsSelect     (Regex* r); // TBD: upcoming feature
  bool     TableRecordsImportSelected();
  bool     TableRecordsImportFromTrigger(Trigger *trigger);
  //
  Record*  RecordNew(bool auto_commit);
  //
  // Record positioning functions.
  Record*  RecordCurrentGet    (          )    { return RecordCurrentGet   (   (ViewIterator*)0); }
  Record*  RecordSearchFirst   (Query* q=0)    { return RecordSearchFirst  (q, (ViewIterator*)0); }
  Record*  RecordSearchNext    (Query* q=0)    { return RecordSearchNext   (q, (ViewIterator*)0); }
  Record*  RecordSearchLast    (Query* q=0)    { return RecordSearchLast   (q, (ViewIterator*)0); }
  Record*  RecordSearchPrev    (Query* q=0)    { return RecordSearchPrev   (q, (ViewIterator*)0); }
  bool     RecordSearchIsLast  (Query* q=0)    { return RecordSearchIsLast (q, (ViewIterator*)0); }
  bool     RecordSearchIsDone  (Query* q=0)    { return RecordSearchIsDone (q, (ViewIterator*)0); }
  Record*  RecordFind          (Query* q=0)    { return RecordFind         (q, (ViewIterator*)0); }
  // Note: We do not allow Search by Condition as Condition objects are mostly
  //       dynamically created and are thus managed (deleted) by a Query object.
  //
  bool     RecordMoveCurrToIter(ViewIterator* iter, eListAddPosition pos, eListCurrentLinkAfterUnlink pos_for_cur);
  bool     RecordMoveIterToCurr(ViewIterator* iter, eListAddPosition pos, eListCurrentLinkAfterUnlink pos_for_iter);
  bool     RecordMoveIterToIter(ViewIterator* itr1, ViewIterator* itr2, eListAddPosition pos, eListCurrentLinkAfterUnlink pos_for_iter);

  // View Iterator API.
  ViewIterator* ViewIteratorNew();
  bool          ViewIteratorFree(ViewIterator* iter);
  bool          PositionSet(ViewIterator* view_iter);
  //
  // TBD: next two delete functions.
  unsigned TranRecordsDelete   (Query* q=0);
  unsigned TranRecordsDelete   (unsigned start=0, unsigned end=0);
  bool     TranRecordAdd       (Record* rec);
  bool     TranRecordDelete    (Record* rec);
  bool     TranRecordModify    (Record* rec, const Attribute* attr, const Value &v);
  //
  bool     RecordAdd           (Record* rec, eListAddPosition pos, ViewIterator* iter=0);
  bool     RecordAddDynamic    (Query *q=0);
  bool     RecordsKeepMax      (unsigned max);

  const Attribute* AttributeFind(const char* attr_name);
  const Attribute* AttributeFind(short index);

  // Aggregates API:
  //////////////////
  Aggregate* AggregateAdd(Attribute *const attr, eAggregateType type);
  // TBD (Aggregates with Query or Regex) next two lines): \
  Aggregate* AggregateAdd(Attribute *const attr, eAggregateType type, Query* q); \
  Aggregate* AggregateAdd(Attribute *const attr, eAggregateType type, Regex* q);
  //
  bool     AggregateDelete(Aggregate* aggr_ptr);
  bool     AggregatesUpdate(eAggregateUpdateReasonType type, Record *rec, const Attribute* attr=0, Value *old_val=0);

  // Trigger API:
  ///////////////
  Trigger* TriggerNew      (const char* trg_name, eTriggerType type, void *trg_owner_obj, TriggerActionFnPtr fn_ptr, Query *q=0, Record *rec=0);
  bool     TriggerDelete   (Trigger *trg);

  // Direct aggregate computation
  void ValueMin(const Attribute* attr, Value *min_val);
  void ValueMax(const Attribute* attr, Value *max_val);

  // View Iterator support.
  void PositionSetSame(ViewIterator* view_iter);

  // View Utilities:
  //////////////////
  // Compare two Views:
  // TBD (View operator == and !=) next paragraph\
  bool     operator==(const View* view);   \
  bool     operator!=(const View* view);   \
  // Sort a View:
  void     Sort(Attribute const* attr, eSortOrder);

  // Print for debug.
  void     Print(ViewIterator* view_iter, int num_of_recs=-1);
  void     Print(int num_of_recs=-1, Record* rec_hgihlight=0);
  void     PrintHeader();
  // TBD:
//bool     ViewContainsRecord(Record* rec);
  unsigned Size() { return mpRecordPtrList->Size(); }

  short AttributesTotal() const;

  List<Record>*          mpRecordPtrList;   // all records selected by view
  unsigned               mRecPermission;

private:
  const char*            mpName;
  Table*                 mpTable;  // the table from which the view was made
//eRecordPermissionBits  mRecPermission;
//List<Record>*          mpRecordPtrList;   // all records selected by view
  List<Aggregate>        mAggregatePtrList; // all aggregates added to view
  List<Record>           mRecordNewPtrList; // all new records, not added yet to table
  List<ViewIterator>     mViewIteratorList;

  // Private member functions:
  View  (const char* view_name, Table* table, unsigned perm);
 ~View ();

public:
  // These are for sole use of ViewIterator (or make ViewIterator a friend of View).
  Record*  RecordCurrentGet    (          ViewIterator* iter);
  Record*  RecordSearchFirst   (Query* q, ViewIterator* iter);
  Record*  RecordSearchNext    (Query* q, ViewIterator* iter);
  Record*  RecordSearchLast    (Query* q, ViewIterator* iter);
  Record*  RecordSearchPrev    (Query* q, ViewIterator* iter);
  bool     RecordSearchIsLast  (Query* q, ViewIterator* iter);
  bool     RecordSearchIsDone  (Query* q, ViewIterator* iter);
  Record*  RecordFind          (Query* q, ViewIterator* iter);
  // Same as above with Query reference instead of pointer.
  Record*  RecordSearchFirst   (Query& q, ViewIterator* iter) { return RecordSearchFirst (&q, iter); }
  Record*  RecordSearchNext    (Query& q, ViewIterator* iter) { return RecordSearchNext  (&q, iter); }
  Record*  RecordSearchLast    (Query& q, ViewIterator* iter) { return RecordSearchLast  (&q, iter); }
  Record*  RecordSearchPrev    (Query& q, ViewIterator* iter) { return RecordSearchPrev  (&q, iter); }
  bool     RecordSearchIsLast  (Query& q, ViewIterator* iter) { return RecordSearchIsLast(&q, iter); }
  bool     RecordSearchIsDone  (Query& q, ViewIterator* iter) { return RecordSearchIsDone(&q, iter); }
  Record*  RecordFind          (Query& q, ViewIterator* iter) { return RecordSearchFirst (&q, iter); }
  // Note: We do not allow Search by Condition as Condition objects are mostly
  //       dynamically created and are thus managed (deleted) by a Query object.
};

class ViewIterator
{
  friend class View;
  friend class ViewJoin;

public:
  // Constructor can only be accessed by View class. Use View class to create an Iterator.

  // Iterator functionality (will simply invoke corresponding View functions.
  Record*  RecordCurrentGet    ()           { return mpView->RecordCurrentGet  (   this); }
  Record*  RecordSearchFirst   (Query* q=0) { return mpView->RecordSearchFirst (q, this); }
  Record*  RecordSearchNext    (Query* q=0) { return mpView->RecordSearchNext  (q, this); }
  Record*  RecordSearchLast    (Query* q=0) { return mpView->RecordSearchLast  (q, this); }
  Record*  RecordSearchPrev    (Query* q=0) { return mpView->RecordSearchPrev  (q, this); }
  bool     RecordSearchIsLast  (Query* q=0) { return mpView->RecordSearchIsLast(q, this); }
  bool     RecordSearchIsDone  (Query* q=0) { return mpView->RecordSearchIsDone(q, this); }
  Record*  RecordFind          (Query* q=0) { return mpView->RecordSearchFirst (q, this); }
  // Same as above with Query reference instead of pointer.
  Record*  RecordSearchFirst   (Query& q) { return mpView->RecordSearchFirst (&q, this); }
  Record*  RecordSearchNext    (Query& q) { return mpView->RecordSearchNext  (&q, this); }
  Record*  RecordSearchLast    (Query& q) { return mpView->RecordSearchLast  (&q, this); }
  Record*  RecordSearchPrev    (Query& q) { return mpView->RecordSearchPrev  (&q, this); }
  bool     RecordSearchIsLast  (Query& q) { return mpView->RecordSearchIsLast(&q, this); }
  bool     RecordSearchIsDone  (Query& q) { return mpView->RecordSearchIsDone(&q, this); }
  Record*  RecordFind          (Query& q) { return mpView->RecordSearchFirst (&q, this); }

  // Set the iterator current link position from another iterator.
  bool PositionSet(ViewIterator* view_iter) {
    return mpRecordListIter->PositionSet(view_iter->mpRecordListIter);
  }

  void PositionSetSameAsView() { mpView->PositionSetSame(this); }

  void Print();

private:
  ListIterator<Record>*  mpRecordListIter;
  View*                  mpView;

  // Private member functions:
  ViewIterator(ListIterator<Record>* rec_iter, View* view) :
    mpRecordListIter (rec_iter),
    mpView           (view)
  { assert(mpView);
    assert(rec_iter);
    M_UpdConstructCount(eType_ViewIterator);
  }

  ~ViewIterator()
  { M_UpdDestructCount(eType_ViewIterator); }
};

#endif

// End
