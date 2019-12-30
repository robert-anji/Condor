// $RCSfile: View.cc,v $
// $Revision: 1.2 $
// 1. Removed function ValueSeparatorGet().
// 2. Removed ValueSeparatorGet().
// 3. Made RecordSearchXXX() functions return the record pointer.

#include <assert.h>

#include "View.h"
//
#include "Aggregate.h"
#include "Record.h"
#include "Table.h"
#include "Trigger.h"

/*! \brief
 * Main View constructor:
 * Requires a view name, the parent table name and permissions.
 * Enjoy!
**/

View::View(const char* name, Table* table, eRecordPermissionBits perm) :
  mpName         (name),
  mpTable        (table),
  mRecPermission (perm)
{
  mpRecordPtrList = new List<Record>;

  assert(mpTable);
  assert(mpName);
  assert(mpRecordPtrList);
}

/**
 *  Main View constructor:
 *  Requires a view name, the parent table name and permissions.
 *  Enjoy!
**/
View::~View()
{
  // Remove references to this view in the Table.
  if (!mpTable->ViewDelete(this)) assert(0);

  // Records are owned by the view's Table and deleted there.
  // Here we need to delete only the link list.
  mpRecordPtrList->DeleteLinkAll(); // XX

  // Clean up new records that were allocated but not added to the table.
  if (mRecordNewPtrList.Size()) {
    MacroListDeleteAll(mRecordNewPtrList, Record);
  }

  MacroListDeleteAll(mAggregatePtrList, Aggregate);

  MacroListDeleteAll(mViewIteratorList, ViewIterator);

  mpTable    = 0;
  mpName     = "";
}

/*! \brief Select records based on a start/end index.
 *
 * This function will select a range
 * of record from the parent table
 * and import them into the view
 */
unsigned View::TableRecordsSelect(unsigned start, unsigned end)
{
  return mpTable->RecordsSelect(this, start, end);
}

unsigned View::TableRecordsSelect(Query* q)
{
  return mpTable->RecordsSelect(this, q);
}

/*! This is used to unselect the View
 * It is not normally used
 * May be used only if unable to obtain import permissions
 */
bool View::ViewRecordUnselect(Record *rec)
{
  // Removing a record from the view is done immediately, not as a transaction
  // because the original record is not removed from the table and thus this
  // action cannot be triggered upon.
  return mpTable->RecordsUnselect(this, rec);
}

bool View::TableRecordsImportSelected()
{
  return mpTable->RecordsImportSelected(this);
}

Record* View::RecordNew()
{
  // Create a new Record but dont set the view yet, that will be done
  // once the record is added to the view/table using RecordAdd() or
  // TranRecordAdd().
  short total_attr = AttributesTotal();

  Record* rec = new Record(total_attr);

  // Update the eValueType of all values in the record.
  for (short i=0; i<total_attr; i++) {
    rec->mpTableRecord->ValueTypeSet(i, mpTable->AttributeType(i));
  }

  if (rec) mRecordNewPtrList.Add(rec);

  return rec;
}

Record* View::RecordCurrentGet(ViewIterator* iter)
{
  if (iter) return iter->mpRecordPtrIter->Get();
  else      return       mpRecordPtrList->Get();
}

Record* View::RecordSearchFirst(Query* q, ViewIterator* iter)
{
  if (iter==0) {
    for (MacroListIterateAll((*mpRecordPtrList)))
    {
      Record* rec = mpRecordPtrList->Get();
      assert(rec);

      if (rec->Match(q)) return rec;
    }
  }
  else {
    for (MacroListIterateAll((*iter->mpRecordPtrIter)))
    {
      Record* rec = iter->mpRecordPtrIter->Get();
      assert(rec);

      if (rec->Match(q)) return rec;
    }
  }

  return 0;
}

Record* View::RecordSearchLast(Query* q, ViewIterator* iter)
{
  if (iter==0) {
    for (MacroListIterateAllReverse((*mpRecordPtrList)))
    {
      Record* rec = mpRecordPtrList->Get();
      assert(rec);

      if (rec->Match(q)) return rec;
    }
  }
  else {
    for (MacroListIterateAllReverse((*iter->mpRecordPtrIter)))
    {
      Record* rec = iter->mpRecordPtrIter->Get();
      assert(rec);

      if (rec->Match(q)) return rec;
    }
  }

  return 0;
}

Record* View::RecordSearchNext(Query* q, ViewIterator* iter)
{
  if (iter) {
    while (!iter->mpRecordPtrIter->IsDone())
    {
      if (iter->mpRecordPtrIter->Next()) {
        Record* rec = iter->mpRecordPtrIter->Get();

        if (rec->Match(q)) return rec;
      }
    }
  }
  else {
    while (!mpRecordPtrList->IsDone())
    {
      if (mpRecordPtrList->Next()) {
        Record* rec = mpRecordPtrList->Get();

        if (rec->Match(q)) return rec;
      }
    }
  }

  return 0;
}

Record* View::RecordSearchPrev(Query* q, ViewIterator* iter)
{
  if (iter) {
    while (!iter->mpRecordPtrIter->IsDone())
    {
      if (iter->mpRecordPtrIter->Prev()) {
        Record* rec = iter->mpRecordPtrIter->Get();

        if (rec->Match(q)) return rec;
      }
    }
  }
  else {
    while (!mpRecordPtrList->IsDone())
    {
      if (mpRecordPtrList->Prev()) {
        Record* rec = mpRecordPtrList->Get();

        if (rec->Match(q)) return rec;
      }
    }
  }

  return 0;
}

bool View::RecordSearchIsLast(Query* q, ViewIterator* iter)
{
  bool is_last;

  if (q == 0) {
    if (iter) is_last = iter->mpRecordPtrIter->IsLast();
    else      is_last =       mpRecordPtrList->IsLast();
  }
  else {
    Record* rec;
    if (iter) iter->mpRecordPtrIter->Get();
    else            mpRecordPtrList->Get();

    // Precondition is that the current record must match the query.
    if (rec->Match(q)) {
      is_last = FALSE;
    }
    else {
      if (iter) iter->mpRecordPtrIter->PositionSave();
      else            mpRecordPtrList->PositionSave();

      is_last = TRUE;
      if (iter) {
        while (! iter->mpRecordPtrIter->IsDone() && is_last) {
          iter->mpRecordPtrIter->Next();

          rec = iter->mpRecordPtrIter->Get();
          if (rec->Match(q)) is_last = FALSE;
        }
      }
      else {
        while (! mpRecordPtrList->IsDone() && is_last) {
          mpRecordPtrList->Next();

          rec = mpRecordPtrList->Get();
          if (rec->Match(q)) is_last = FALSE;
        }
      }

      if (iter) iter->mpRecordPtrIter->PositionRestore();
      else            mpRecordPtrList->PositionRestore();
    }
  }

  return is_last;
}

bool View::RecordSearchIsDone(Query* q, ViewIterator* iter)
{
  bool is_done;
  bool q_match;

  if (q == 0) {
    if (iter) is_done = iter->mpRecordPtrIter->IsDone();
    else      is_done =       mpRecordPtrList->IsDone();
  }
  else {
    Record* rec;

    if (iter) rec = iter->mpRecordPtrIter->Get();
    else      rec =       mpRecordPtrList->Get();

    if (!rec) return TRUE; // return TRUE if list is empty

    // If the current record matches then we are not done.
    if (rec->Match(q)) {
      is_done = FALSE;
    }
    else {
      if (iter) iter->mpRecordPtrIter->PositionSave();
      else            mpRecordPtrList->PositionSave();

      q_match = FALSE;
      if (iter) {
        while (! iter->mpRecordPtrIter->IsDone() && !q_match) {
          mpRecordPtrList->Next();

          rec = iter->mpRecordPtrIter->Get();

          if (rec->Match(q)) q_match = TRUE;
        }
      }
      else {
        while (! mpRecordPtrList->IsDone() && !q_match) {
          mpRecordPtrList->Next();

          rec = mpRecordPtrList->Get();

          if (rec->Match(q)) q_match = TRUE;
        }
      }
      is_done = !q_match;

      if (iter) iter->mpRecordPtrIter->PositionRestore();
      else            mpRecordPtrList->PositionRestore();
    }
  }

  return is_done;
}

// Non-transactional record add. We can allow this when a Table has only 1 view.
// The record is also immediately added to the view.
// Currently, to add a record only to the table and not to the view
// TranRecordAdd() must be used accompanied by a Trigger that selects
// the records that should be added to the view.
bool View::RecordAdd(Record* &rec, eListAddPosition pos)
{
  bool ret;

  // First we remove the allocated record from the 'new' list.
  MacroListDeleteLink(mRecordNewPtrList, rec, ret);

  if (ret) {
    // Create a new Record.
    Record* rec_add = new Record(rec->mpTableRecord);

    // rec->mpTableRecord pointer is now transferred and stored in rec_add,
    // set rec->mpTableRecord to 0 to protect the pointer from rec's destructor.
    rec->mpTableRecord = 0;

    delete rec; rec = 0;

    // Now add it to the table and if successful then to the view as well.
    // Note that there is no 'pos' for a table record add.
    if (mpTable->RecordAdd(this, rec_add)) {
      rec_add->mpView = this;
       return mpRecordPtrList->Add(rec_add, pos);
    }
  }
  else assert(0);

  return FALSE;
}

// Transactional record add.
bool View::TranRecordAdd(Record* &rec)
{
  bool    ret;

  // First we remove the allocated record from the 'new' list.
  MacroListDeleteLink(mRecordNewPtrList, rec, ret);

  if (ret) {
    // Create a new Record.
    Record* rec_add = new Record(rec->mpTableRecord);

    // rec->mpTableRecord pointer is now transferred and stored in rec_add,
    // set rec->mpTableRecord to 0 to protect the pointer from rec's destructor.
    rec->mpTableRecord = 0;

    delete rec; rec = 0;

    // Now we add it as a transaction for the table.
    return mpTable->TranRecordAdd(this, rec_add);
  }
  else assert(0);

  return FALSE;
}

bool View::TranRecordDelete(Record *rec)
{
  return mpTable->TranRecordDelete(this, rec);
}

bool View::TranRecordModify(Record* rec, const Attribute* attr, const Value& val)
{
  return mpTable->TranRecordModify(this, rec, attr, val);
}

bool View::TableRecordsImportFromTrigger(Trigger *trigger)
{
  mpTable->RecordImportFromTrigger(trigger);
}

void TriggerActionFN_ForRecAdd(Trigger *trigger)
{
  (trigger->ViewGet())->TableRecordsImportFromTrigger(trigger);
}

bool View::RecordAddDynamic(Query *q)
{
  // 1. View adds a trigger for record-add and supplies a trigger-action function
  //    along with a trigger query that must be satisfied.
  // 2. The trigger-action function will receive the trigger object which contains
  //    a reference to the just added record (it is private, accesible by Table).
  // 3. The trigger-action function should import the record by invoking an import
  //    function in the Table class and supplying the trigger object so that the
  //    Table class can know the record that needs importing.
  Record  *rec            = 0;
  void    *trg_owner_obj  = 0;
  Trigger *trg;

  trg = mpTable->TriggerNew(eTrgRecAdd, trg_owner_obj, this, q, rec);
  mpTable->TriggerActivate(trg, TriggerActionFN_ForRecAdd);

  return TRUE;
}

bool View::RecordsKeepMax(unsigned max)
{
  Aggregate* aggr = new Aggregate(this, eAggr_RecordsCount, 0);
  assert(aggr);

  Record  *rec            = 0;
  void    *trg_owner_obj  = 0;
  Trigger *trg;
  Query   *q;

  // CONTOLD: Add TriggerActionFN_RecordsKeepMax and modify query to allow comparison with aggr.
//trg = mpTable->TriggerAdd(eTrgRecAdd, TriggerActionFN_RecordsKeepMax, trg_owner_obj, this, aggr, q);
}

const Attribute* View::AttributeFind(const char* attr_name)
{
  return mpTable->AttributeFind(attr_name);
}

const Attribute* View::AttributeFind(short index)
{
  return mpTable->AttributeFind(index);
}

short View::AttributesTotal() const
{
  return mpTable->AttributesTotal();
}

Aggregate* View::AggregateAdd(Attribute *const attr, eAggregateType type)
{
  Aggregate *aggr = new Aggregate(this, type, attr);

  assert(aggr);

  mAggregatePtrList.Add(aggr);
}

bool View::AggregateDelete(Aggregate* aggr)
{
  bool ret;

  if (aggr) {
    MacroListDeleteElm(mAggregatePtrList, aggr, ret);
  }
  else
    assert(0);

  assert(ret);

  return ret;
}

bool View::AggregatesUpdate(eAggregateUpdateReasonType rsn_type, Record *rec, const Attribute* mdfy_attr, Value *old_val)
{
  Aggregate        *aggr;
  Value             changed_val;
  printf("VALUE:%s %d\n", __FILE__, __LINE__);


  if (mAggregatePtrList.Size()) {
    for (MacroListIterateAll(mAggregatePtrList)) {
      aggr      = mAggregatePtrList.Get();
      assert(aggr);

      const Attribute*  aggr_attr = aggr->AttributeGet();
      assert(aggr_attr);

      // Modify affects a single attribute specified by 'mdfy_attr' passed in
      // and will affect only the corresponding aggregates, whereas 'Add' and
      // 'Delete' will affect all attributes and hence all aggregates.
      if (rsn_type == eAggrUpdType_RecordModify) {
        if (aggr_attr == mdfy_attr) rec->ValueCopyGet(mdfy_attr, changed_val);
        else                        continue;
      }
      else {
        rec->ValueCopyGet(aggr_attr, changed_val);
      }

      aggr->ValueUpdate(rsn_type, &changed_val, old_val);
    }
  }

  return TRUE;
}

Trigger* View::TriggerNew(eTriggerType type, void *trg_owner_obj, Query *q, Record *rec)
{
  Trigger *trg = mpTable->TriggerNew(type, trg_owner_obj, this, q, rec);

  return trg;
}

Trigger* View::TriggerNew(eTriggerType type, void *trg_owner_obj, Aggregate *aggr, Query *q)
{
  Trigger *trg = mpTable->TriggerNew(type, trg_owner_obj, this, aggr, q);
  assert(trg);

  return trg;
}

bool View::TriggerActivate(Trigger* trg, TriggerActionFnPtr fn)
{
  return mpTable->TriggerActivate(trg, fn);
}

bool View::TriggerActivate(Trigger* trg, Process* process)
{
  return mpTable->TriggerActivate(trg, process);
}

bool View::TriggerDelete(Trigger *trg)
{
  return mpTable->TriggerDelete(trg);
}

bool View::TriggerMatched(Trigger *trg)
{
  return mpTable->TriggerDelete(trg);
}

void View::ValueMin(const Attribute* attr, Value *min_val)
{
  assert(min_val);

  min_val->SetToMax();

  for (MacroListIterateAll((*mpRecordPtrList))) {
    Record *rec = mpRecordPtrList->Get();

    if ((*rec)[attr] < *min_val) {
      rec->ValueCopyGet(attr, *min_val);
    }
  }
}

void View::ValueMax(const Attribute* attr, Value *max_val)
{
  assert(max_val);

  max_val->SetToMin();

  for (MacroListIterateAll((*mpRecordPtrList))) {
    Record *rec = mpRecordPtrList->Get();

    if ((*rec)[attr] > *max_val) {
      rec->ValueCopyGet(attr, *max_val);
    }
  }
}

void View::Sort(Attribute const* attr, eSortOrder sort_order)
{
  List<Record>*  list_sorted = mpRecordPtrList;
  List<Record>   list_unsorted;
  Record*        rec_unsorted;
  Record*        rec_sorted;
  bool           found;

  if (mpRecordPtrList->First() == FALSE) return; // no records to sort
  if (mpRecordPtrList->IsLast())         return; // only 1 record, no sort needed.

  list_sorted->First();

  // Splits after the first record, now list_sorted will have
  // one record while list_unsorted will have the remaining.
  //
  list_sorted->Split(list_unsorted, eList_SplitAfterIterator);

  // The following does an inline sort, i.e. only the link positions
  // are altered, no new links are created or deleted in the process.
  // The sort order is 'stable' (shouldnt matter as the list contains pointers).

  if (sort_order == eSortOrder_Ascending) {
    for (MacroListIterateAll(list_unsorted)) {
      rec_unsorted = list_unsorted.Get();

      found = FALSE;
      for (MacroListIterateAll(*list_sorted)) {
        rec_sorted = list_sorted->Get();

        if ((*rec_unsorted)[attr] < (*rec_sorted)[attr]) {
          list_sorted->LinkTransferFrom(list_unsorted, eListLinkTransferType_FromCurrentToBeforeCurrent);
          found = TRUE;
          break;
        }
      }
      if (!found)
        list_sorted->LinkTransferFrom(list_unsorted, eListLinkTransferType_FromCurrentToAfterCurrent);
    }
  }

  if (sort_order == eSortOrder_Descending) {
    for (MacroListIterateAll(list_unsorted)) {
      rec_unsorted = list_unsorted.Get();

      found = FALSE;
      for (MacroListIterateAll(*list_sorted)) {
        rec_sorted = list_sorted->Get();

        if ((*rec_unsorted)[attr] > (*rec_sorted)[attr]) {
          list_sorted->LinkTransferFrom(list_unsorted, eListLinkTransferType_FromCurrentToBeforeCurrent);
          found = TRUE;
          break;
        }
      }
      if (!found)
        list_sorted->LinkTransferFrom(list_unsorted, eListLinkTransferType_FromCurrentToAfterCurrent);
    }
  }
}

ViewIterator* View::ViewIteratorNew()
{
  ListIterator<Record>* rec_iter = mpRecordPtrList->ListIteratorNew();
  assert(rec_iter);

  ViewIterator* iter = new ViewIterator(rec_iter, this);

  mViewIteratorList.Add(iter);

  return iter;
}

bool View::ViewIteratorFree(ViewIterator* iter)
{
  bool success;

  mpRecordPtrList->ListIteratorDelete(iter->mpRecordPtrIter);

  MacroListDeleteElm(mViewIteratorList, iter, success);

  return success;
}

bool View::PositionSet(ViewIterator* view_iter)
{
  return mpRecordPtrList->PositionSet(view_iter->mpRecordPtrIter);
}

/** \brief Print the position of the view iterator
 * kjfkdsjfkd
 *  kdfjdlkjflsdkj
 *  kdfjdlkjflsdkj
 */
void View::Print(unsigned num_of_recs)
{
  if (num_of_recs == 0) {
    // Print all records.
    mpRecordPtrList->PositionSave();

    printf("View records are:\n");
    for (MacroListIterateAll((*mpRecordPtrList))) {
      Record *rec = mpRecordPtrList->Get();

      if (rec) rec->Print();
      else     assert(0);
    }
  }

  mpRecordPtrList->PositionRestore();

  if (num_of_recs==0 || num_of_recs==1) {
    printf("View current Record is:\n");
    Record *rec = mpRecordPtrList->Get();
    if (rec) rec->Print();
  }

  if (num_of_recs!=0 && num_of_recs!=1) {
    // Not yet implemented, print 'num_of_recs' from current record.
    assert(0);
  }
}

void ViewIterator::Print()
{
  Record* rec = RecordCurrentGet();
  assert(rec);
  rec->Print();
}

// End
