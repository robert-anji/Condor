// $RCSfile: View.cc,v $
// $Revision: 1.10 $

#include <assert.h>

#include "View.h"
//
#include "Aggregate.h"
#include "Global.h"
#include "List.h"
#include "Query.h"
#include "Record.h"
#include "Table.h"
#include "Trigger.h"

M_ExternConstructDestructCountInt;

View::View(const char* name, Table* table, unsigned perm) :
  mpName         (name),
  mpTable        (table),
  mRecPermission (perm)
{
  mpRecordPtrList = new List<Record>;

  assert(mpTable);
  assert(mpName);
  assert(mpRecordPtrList);

  M_UpdConstructCount(eType_View);
}

View::~View()
{
  // The view deletes all the records. The Record destructor currently
  // does not delete the mpTableRecord in Record class.
  mpRecordPtrList = mpRecordPtrList;
  MacroListDeleteAll((*mpRecordPtrList), Record);

  delete mpRecordPtrList; mpRecordPtrList = 0;

  // Clean up new records that were allocated but not added to the table.
  if (mRecordNewPtrList.Size()) {
    MacroListDeleteAll(mRecordNewPtrList, Record);
  }

  MacroListDeleteAll(mAggregatePtrList, Aggregate);

  MacroListDeleteAll(mViewIteratorList, ViewIterator);

  printf("Destruct:View:%s\n", mpName);

  mpTable    = 0;
  mpName     = "";

  M_UpdDestructCount(eType_View);
}

unsigned View::TableRecordsSelect(unsigned start, unsigned end)
{
  return mpTable->RecordsSelect(this, start, end);
}

unsigned View::TableRecordsSelect(Query* q)
{
  return mpTable->RecordsSelect(this, q);
}

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

Record* View::RecordNew(bool allow_val_update_before_commit)
{
  // Create a new Record but dont set the view yet, that will be done
  // once the record is added to the view/table using RecordAdd() or
  // TranRecordAdd().
  short total_attr = AttributesTotal();

  Record* rec = new Record(total_attr, allow_val_update_before_commit);

  if (rec) {
    // Update the eValueType of all values in the record.
    for (short i=0; i<total_attr; i++) {
      rec->mpTableRecord->ValueTypeSet(i, mpTable->AttributeType(i));
    }

    mRecordNewPtrList.Add(rec);
  }

  return rec;
}

Record* View::RecordCurrentGet(ViewIterator* iter)
{
  Record* rec;

  if (iter) rec = iter->mpRecordListIter->Get();
  else      rec =       mpRecordPtrList->Get();

//if (rec) rec->Print();
//else     printf("NULL rec\n");

  return rec;
}

Record* View::RecordSearchFirst(Query* q, ViewIterator* iter)
{
  if (iter==0) {
    for (MacroListIterateAll((*mpRecordPtrList)))
    {
      Record* rec = mpRecordPtrList->Get();
      assert(rec);

      if (q) { if (q->Match(rec)) return rec; }
      else   {                    return rec; }
    }
  }
  else {
    for (MacroListIterateAll((*iter->mpRecordListIter)))
    {
      Record* rec = iter->mpRecordListIter->Get();
      assert(rec);

      if (q) { if (q->Match(rec)) return rec; }
      else   {                    return rec; }
    }
  }

  return (Record*)0;
}

Record* View::RecordSearchLast(Query* q, ViewIterator* iter)
{
  if (iter==0) {
    for (MacroListIterateAllReverse((*mpRecordPtrList)))
    {
      Record* rec = mpRecordPtrList->Get();
      assert(rec);

      if (q) { if (q->Match(rec)) return rec; }
      else   {                    return rec; }
    }
  }
  else {
    for (MacroListIterateAllReverse((*iter->mpRecordListIter)))
    {
      Record* rec = iter->mpRecordListIter->Get();
      assert(rec);

      if (q) { if (q->Match(rec)) return rec; }
      else   {                    return rec; }
    }
  }

  return 0;
}

Record* View::RecordSearchNext(Query* q, ViewIterator* iter)
{
  if (iter) {
    while (!iter->mpRecordListIter->IsDone())
    {
      if (iter->mpRecordListIter->Next()) {
        Record* rec = iter->mpRecordListIter->Get();

        if (q) { if (q->Match(rec)) return rec; }
        else   {                    return rec; }
      }
    }
  }
  else {
    while (!mpRecordPtrList->IsDone())
    {
      if (mpRecordPtrList->Next()) {
        Record* rec = mpRecordPtrList->Get();

        if (q) { if (q->Match(rec)) return rec; }
        else   {                    return rec; }
      }
    }
  }

  return 0;
}

Record* View::RecordSearchPrev(Query* q, ViewIterator* iter)
{
  if (iter) {
    while (!iter->mpRecordListIter->IsDone())
    {
      if (iter->mpRecordListIter->Prev()) {
        Record* rec = iter->mpRecordListIter->Get();

        if (q) { if (q->Match(rec)) return rec; }
        else   {                    return rec; }
      }
    }
  }
  else {
    while (!mpRecordPtrList->IsDone())
    {
      if (mpRecordPtrList->Prev()) {
        Record* rec = mpRecordPtrList->Get();

        if (q) { if (q->Match(rec)) return rec; }
        else   {                    return rec; }
      }
    }
  }

  return 0;
}

bool View::RecordSearchIsLast(Query* q, ViewIterator* iter)
{
  bool is_last;

  if (q == 0) {
    if (iter) is_last = iter->mpRecordListIter->IsLast();
    else      is_last =       mpRecordPtrList->IsLast();
  }
  else {
    Record* rec;
    if (iter) iter->mpRecordListIter->Get();
    else            mpRecordPtrList->Get();

    // Precondition is that the current record must match the query.
    if (q->Match(rec)) {
      is_last = FALSE;
    }
    else {
      if (iter) iter->mpRecordListIter->PositionSave();
      else            mpRecordPtrList->PositionSave();

      is_last = TRUE;
      if (iter) {
        while (! iter->mpRecordListIter->IsDone() && is_last) {
          iter->mpRecordListIter->Next();

          rec = iter->mpRecordListIter->Get();
          if (q->Match(rec)) is_last = FALSE;
        }
      }
      else {
        while (! mpRecordPtrList->IsDone() && is_last) {
          mpRecordPtrList->Next();

          rec = mpRecordPtrList->Get();
          if (q->Match(rec)) is_last = FALSE;
        }
      }

      if (iter) iter->mpRecordListIter->PositionRestore();
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
    if (iter) is_done = iter->mpRecordListIter->IsDone();
    else      is_done =       mpRecordPtrList->IsDone();
  }
  else {
    Record* rec;

    if (iter) rec = iter->mpRecordListIter->Get();
    else      rec =       mpRecordPtrList->Get();

    if (!rec) return TRUE; // return TRUE if list is empty

    // If the current record matches then we are not done.
    if (q->Match(rec)) {
      is_done = FALSE;
    }
    else {
      if (iter) iter->mpRecordListIter->PositionSave();
      else            mpRecordPtrList->PositionSave();

      q_match = FALSE;
      if (iter) {
        while (! iter->mpRecordListIter->IsDone() && !q_match) {
          mpRecordPtrList->Next();

          rec = iter->mpRecordListIter->Get();

          if (q->Match(rec)) q_match = TRUE;
        }
      }
      else {
        while (! mpRecordPtrList->IsDone() && !q_match) {
          mpRecordPtrList->Next();

          rec = mpRecordPtrList->Get();

          if (q->Match(rec)) q_match = TRUE;
        }
      }
      is_done = !q_match;

      if (iter) iter->mpRecordListIter->PositionRestore();
      else            mpRecordPtrList->PositionRestore();
    }
  }

  return is_done;
}

Record* View::RecordFind(Query* q, ViewIterator* iter)
{
  return RecordSearchFirst(q, iter);
}

bool View::RecordMoveCurrToIter(ViewIterator* iter, eListAddPosition pos_add, eListCurrentLinkAfterUnlink pos_for_cur)
{
  return mpRecordPtrList->RelinkTo(iter->mpRecordListIter, pos_add, pos_for_cur);
}

bool View::RecordMoveIterToCurr(ViewIterator* iter, eListAddPosition pos_add, eListCurrentLinkAfterUnlink pos_for_unlink)
{
  return mpRecordPtrList->RelinkFrom(iter->mpRecordListIter, pos_add, pos_for_unlink);
}

bool View::RecordMoveIterToIter(ViewIterator* itr1, ViewIterator* itr2, eListAddPosition pos_add, eListCurrentLinkAfterUnlink pos_for_unlink)
{
  return mpRecordPtrList->RelinkFromTo(itr1->mpRecordListIter, itr2->mpRecordListIter, pos_add, pos_for_unlink);
}

// Non-transactional record add. We can allow this when a Table has only 1 view.
// The record is also immediately added to the view.
// Currently, to add a record only to the table and not to the view
// TranRecordAdd() must be used accompanied by a Trigger that selects
// the records that should be added to the view.
bool View::RecordAdd(Record* rec_add, eListAddPosition pos, ViewIterator* iter)
{
  bool ret;

  // First we find and remove the link containing the record.
  MacroListDeleteLink(mRecordNewPtrList, rec_add, ret);

  if (ret) {
  //Record* rec_add = new Record(rec->mpTableRecord);

  //delete rec; rec = 0;

    // Now add it to the table and if successful then to the view as well.
    // Note that there is no 'pos' for a table record add.
    if (mpTable->RecordAdd(this, rec_add)) {
      rec_add->mpView = this;

      if (iter) return mpRecordPtrList->Add(rec_add, pos, iter->mpRecordListIter);
      else      return mpRecordPtrList->Add(rec_add, pos);
    }
    else assert(0);
  }
  else {
    assert(0);
  }

  return FALSE;
}

// Transactional record add.
bool View::TranRecordAdd(Record* rec)
{
  bool    ret;

  // First we remove the allocated record from the 'new' list.
  MacroListDeleteLink(mRecordNewPtrList, rec, ret);

  if (ret) {
    Record* rec_add = new Record(rec->mpTableRecord);

    delete rec; rec = 0;

    // Now we add it as a transaction for the table.
    return mpTable->TranRecordAdd(this, rec_add);
  }
  else {
    assert(0);
  }

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

  trg = mpTable->TriggerNew("RecordAddDynamic", eTrgRecAdd, trg_owner_obj, TriggerActionFN_ForRecAdd, this, q, rec);

  return TRUE;
}

bool View::RecordsKeepMax(unsigned max)
{
  Aggregate* aggr = new Aggregate(this, eAggr_RecordsCount, 0);
  assert(aggr);

  Record  *rec            = 0;
  void    *trg_owner_obj  = 0;
  Trigger *trg;
  Query   *q; // c = new Condition(aggr); q = (c == max);

  mAggregatePtrList.Add(aggr);
  // TBD Sep 23 2008: Add TriggerActionFN_RecordsKeepMax, allow query comparison with aggr.
  //trg = mpTable->TriggerNew(eTrgRecAdd, TriggerActionFN_RecordsKeepMax, trg_owner_obj, this, aggr, q);
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

bool View::AggregatesUpdate(eAggregateUpdateReasonType rsn_type, Record *rec, const Attribute* attr_mdfy, Value *old_val)
{
  Aggregate        *aggr;
  Value             changed_val; M_Constructed(Value);

  if (mAggregatePtrList.Size()) {
    for (MacroListIterateAll(mAggregatePtrList)) {
      aggr      = mAggregatePtrList.Get();
      assert(aggr);

      const Attribute*  attr_aggr = aggr->AttributeGet();
      assert(attr_aggr);

      // Modify affects a single attribute specified by 'attr_mdfy' passed in
      // and will affect only the corresponding aggregates, whereas 'Add' and
      // 'Delete' will affect all attributes and hence all aggregates.
      if (rsn_type == eAggrUpdType_RecordModify
      ||  rsn_type == eAggrUpdType_RecordModifyPropagate)
      {
        if (attr_aggr == attr_mdfy) rec->ValueCopyGet(attr_mdfy, changed_val);
        else                        continue;
      }
      else {
        rec->ValueCopyGet(attr_aggr, changed_val);
      }

      aggr->ValueUpdate(rsn_type, &changed_val, old_val);

      // If rsn_type is 'modify' and the records reference count is more than 1
      // then aggregates in other views may need to be updated, this needs to be
      // done by the views table, reference count is private & checked by table.
      if (rsn_type == eAggrUpdType_RecordModify) {
        mpTable->AggregatesUpdate(this, rsn_type, rec, attr_mdfy, old_val);
      }
    }
  }

  return TRUE;
}

Trigger* View::TriggerNew(const char* trg_name, eTriggerType type, void *trg_owner_obj, TriggerActionFnPtr fn_ptr, Query *q, Record *rec)
{
  assert(0);
  Trigger *trg = mpTable->TriggerNew(trg_name, type, trg_owner_obj, fn_ptr, this, q, rec);

  return trg;
}

bool View::TriggerDelete(Trigger *trg)
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
          list_sorted->RelinkFrom(&list_unsorted, eListAddPosition_BeforeCurrent, eListCurrentLinkAfterUnlink_Next);
          found = TRUE;
          break;
        }
      }
      if (!found)
        list_sorted->RelinkFrom(&list_unsorted, eListAddPosition_AfterCurrent, eListCurrentLinkAfterUnlink_Next);
    }
  }

  if (sort_order == eSortOrder_Descending) {
    for (MacroListIterateAll(list_unsorted)) {
      rec_unsorted = list_unsorted.Get();

      found = FALSE;
      for (MacroListIterateAll(*list_sorted)) {
        rec_sorted = list_sorted->Get();

        if ((*rec_unsorted)[attr] > (*rec_sorted)[attr]) {
          list_sorted->RelinkFrom(&list_unsorted, eListAddPosition_BeforeCurrent, eListCurrentLinkAfterUnlink_Next);
          found = TRUE;
          break;
        }
      }
      if (!found)
        list_sorted->RelinkFrom(&list_unsorted, eListAddPosition_AfterCurrent, eListCurrentLinkAfterUnlink_Next);
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

  mpRecordPtrList->ListIteratorDelete(iter->mpRecordListIter);

  MacroListDeleteElm(mViewIteratorList, iter, success);

  return success;
}

bool View::PositionSet(ViewIterator* view_iter)
{
  return mpRecordPtrList->PositionSet(view_iter->mpRecordListIter);
}

void View::PositionSetSame(ViewIterator* view_iter)
{
  view_iter->mpRecordListIter->PositionSet(mpRecordPtrList);
}

void View::Print(ViewIterator* view_iter, int num_of_recs)
{
  Record* rec;

  if (view_iter) rec = view_iter->RecordCurrentGet();
  else           rec = 0;

  return Print(num_of_recs, rec);
}

void View::Print(int num_of_recs, Record* rec_highlight)
{
  Record* rec;

  if (num_of_recs == -1) {
    // Print all records.
    if (rec_highlight==0) rec_highlight=RecordCurrentGet();

    mpRecordPtrList->PositionSave();

    for (MacroListIterateAll((*mpRecordPtrList))) {
      Record *rec = mpRecordPtrList->Get();

      if (rec) {
        if (rec == rec_highlight) printf("@");
        else                      printf(" ");

        printf("rec=%p: ", rec);
        rec->Print();
      }
      else     assert(0);
    }

    mpRecordPtrList->PositionRestore();
  }

  if (num_of_recs==0) {
    Record *rec = mpRecordPtrList->Get();
    if (rec) printf("rec=%p: ", rec);
    if (rec) rec->Print();
  }
}

void View::PrintHeader()
{
  mpTable->PrintAttributeNames();
}

void ViewIterator::Print()
{
  Record* rec_current;
  Record* rec;

  rec_current = RecordCurrentGet();

  if (!rec_current) return; // no records in the iterator

  mpRecordListIter->PositionSave();

  for (MacroListIterateAll((*mpRecordListIter))) {
    rec = mpRecordListIter->Get();
    printf("rec=%p: ", rec);

    if (rec) {
      if (rec == rec_current) printf("->");
      else                    printf("  ");

      rec->Print();
    }
    else     assert(0);
  }

  mpRecordListIter->PositionRestore();
}

// End
