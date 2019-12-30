//  $RCSfile: Table.cc,v $
//  $Revision: 1.10 $

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "Table.h"
//
#include "Attribute.h"
#include "Bit.h"
#include "Database.h"
#include "Global.h"
#include "List.h"
#include "Query.h"
#include "Record.h"
#include "Scheduler.h"
#include "TableRecord.h"
#include "Trigger.h"
#include "Value.h"

extern Scheduler* gScheduler;
M_ExternConstructDestructCountInt;

static const char* gDefaultDatabaseDirectory = "./";
static const char* gRecordSeparator          = "\n";
static const char* gAttributeSeparator       = ">|<";

Table::Table(Database* db, const char* tbname, int flags) :
  mpDatabase         (db),
  mpName             (tbname),
  mTableOpenFlags    (flags),
  mpViewOwner        (0),
  mViewOwnerRecPermission(eRecPerm_None),
  mpAttr_RecordPtr   (0),
  mpAttr_RecordCount (0)
{
  printf("Creating table: %s", mpName), M_FL;
  if (mTableOpenFlags & eTableOpenFlagBit_FromFile) Read(flags);

  mpAttr_RecordPtr   = new Attribute("TableRecordPtr");
  mpAttr_RecordCount = new Attribute("TableRecordCount");

  M_UpdConstructCount(eType_Table);
}

// TBD: Remove this, operation is not correct or safe and use-case is unknown.
Table::Table(Table& tbl)
{
  *this = tbl;

  M_UpdConstructCount(eType_Table);
}

Table::~Table()
{
  MacroListDeleteAll(mAttributeList,           Attribute);
  MacroListDeleteAll(mTableRecordList,         TableRecord);
  MacroListDeleteAll(mTriggerNewList,          Trigger);
  MacroListDeleteAll(mTriggerRecordAddList,    Trigger);
  MacroListDeleteAll(mTriggerRecordDeleteList, Trigger);
  MacroListDeleteAll(mTriggerRecordModifyList, Trigger);
  MacroListDeleteAll(mTriggerAggregateList,    Trigger);

  if (mViewInfoList.Size()) {
    // Maybe change to warning, clients must use ViewDelete() on all views.
    //assert(0); // exe hang if uncommented, 07-10-09

    for (MacroListIterateAll(mViewInfoList)) {
      ViewInfo *view_info = mViewInfoList.Get();
      assert(view_info);

      delete (view_info);
    }
  }

  if (mpAttr_RecordPtr  ) { delete mpAttr_RecordPtr;   mpAttr_RecordPtr   = 0; }
  if (mpAttr_RecordCount) { delete mpAttr_RecordCount; mpAttr_RecordCount = 0; }

  // Remaining lists will automatically clean up in their destructors.

  printf("Destruct:Table:%s\n", mpName);

  M_UpdDestructCount(eType_Table);
}

Attribute* Table::AttributeAdd(const char* attr_name, eValueType val_type, Value *null_val, short width)
{
  Attribute *attr = 0;

  // For now not allowed to add attributes after records exist.
  assert (mTableRecordList.Size()==0);

  // We use SHRT_MAX as an invalid position so reserve that value.
  if (mAttributeList.Size() < SHRT_MAX) {
    attr = new Attribute(attr_name, val_type, null_val, width);
    assert (attr);

    attr->mpTable = this;

    attr->PositionSet(mAttributeList.Size()&SHRT_MAX);

    mAttributeList.Add(attr, eListAddPosition_Last);
  }

  return attr;
}

bool Table::AttributesAdd(TableAttributes* attrs)
{
  Attribute *attr;
  int        count;

  if (attrs == 0) return FALSE;

  // For now not allowed to add attributes after records exist.
  assert (mTableRecordList.Size()==0);
  
  for (count=0; attrs[count].mAttributeType != eValueUnknown; count++) {
    if (!AttributeAdd(attrs[count].mAttributeName, attrs[count].mAttributeType))
      return FALSE;
  }

  return TRUE;
}

//bool Table::TableAdd(Table& tbl)
//{
//  // TBD: Compare List, attribute names and types should match.
//
//  // TBD: Until we have List::Last(), use this:
//  for (mTableRecordList.First(); ! mTableRecordList.IsDone(); mTableRecordList.Next());
//
//  for (tbl.mTableRecordList.First(); ! tbl.mTableRecordList.IsDone(); tbl.mTableRecordList.Next())
//  {
//    Record* rec = tbl.mTableRecordList.GetRecord();
//
//    if (rec) mTableRecordList.Add(new Record(*rec));
//  }
//
//  return TRUE;
//}

View* Table::ViewCreate(const char* view_name, unsigned perm)
{
  View* view = new View(view_name, this, perm);

  if (view) {
    ViewInfo *view_info = new ViewInfo(view, perm);
    assert(view_info);

    mViewInfoList.Add(view_info);
  }

  return view;
}

bool Table::ViewDelete(View* view)
{
  // Delete the view from the appropriate location.
  if (mpViewOwner == view) {
    delete view; view = 0;

    mpViewOwner = 0; 

    if (mViewListGroup.Size()) {
      // Leave mViewOwnerRecPermission unchanged
    }
    else {
      mViewOwnerRecPermission = eRecPerm_None;
    }
  }
  else if (mViewListGroup.PositionFind(view)) {
    View* view =  mViewListGroup.Get();
    assert(view);
    delete view;
    mViewListGroup.DeleteLinkThenIterationDone();
  }
}

// To qualify to belong to the views 'group' the view must have the
// handle to the owner view.
bool Table::ViewGroupAdd(View* view_for_group, View* owner_view)
{
  if (mpViewOwner!=0 && mpViewOwner==owner_view) {
    mViewListGroup.Add(view_for_group);
    return TRUE;
  }
  else {
    assert(0);
    return FALSE;
  }
}

unsigned Table::RecordsSelect(const View* view, unsigned start, unsigned end)
{
  unsigned select_count = 0;

  // Sanity checks.
  if (start >= mTableRecordList.Size()
  ||  end   >= mTableRecordList.Size())
  {
    assert(0);
    return select_count=0;
  }

  RecordsSelectedByView* rec_sel_by_view;

  // If first time the view is selecting some records,
  // then create a new record list for the view.
  bool found = FALSE;
  for (MacroListIterateAll(mRecordListSelectedByViewList))
  {
    rec_sel_by_view = mRecordListSelectedByViewList.Get();

    if (rec_sel_by_view) {
      if (rec_sel_by_view->mpView != view) { continue;     }
      else                                 { found = TRUE; }
    }
    else
      assert(0);
  }
  //
  if (!found) {
    rec_sel_by_view = new RecordsSelectedByView;

    if (rec_sel_by_view) {
      rec_sel_by_view->mpView       = view;
      rec_sel_by_view->mpRecordList = new List<Record>;

      if (rec_sel_by_view->mpRecordList) {
        mRecordListSelectedByViewList.Add(rec_sel_by_view);
      }
      else
        assert(0);
    }
    else assert(0);
  }
  else {
    rec_sel_by_view = mRecordListSelectedByViewList.Get();
  }

  // Now add the records specified to the view.
  if (start <= end) {
    if (mTableRecordList.PositionSet(start)) {
      unsigned count = 0;
      unsigned total = end - start + 1; // this will not support MAX_UNSIGNED

      while (count < total) {
        TableRecord* rec = mTableRecordList.Get();
        rec_sel_by_view->mpRecordList->Add(new Record(rec));

        mTableRecordList.Next();
        count++;
      }

      select_count = count;
    }
  }
  else {
    assert (0); // maybe later, support both directions
  }

  return select_count;
}

unsigned Table::RecordsSelect(const View* view, Query* q)
{
  unsigned select_count = 0;

  RecordsSelectedByView* rec_sel_by_view;

  // Check if there already is a list of selected records.
  bool found = FALSE;
  for (MacroListIterateAll(mRecordListSelectedByViewList)) {
    rec_sel_by_view = mRecordListSelectedByViewList.Get();

    assert(rec_sel_by_view);

    if (rec_sel_by_view->mpView == view) { found = TRUE; break; }
  }

  if (! found) {
    // If it is the first time the view is selecting some records,
    // then create a new record list for the view.
    rec_sel_by_view = new RecordsSelectedByView;

    if (rec_sel_by_view) {
      rec_sel_by_view->mpView       = view;
      rec_sel_by_view->mpRecordList = new List<Record>;

      if (rec_sel_by_view->mpRecordList) {
        mRecordListSelectedByViewList.Add(rec_sel_by_view);
      }
      else
        assert(0);
    }
    else assert(0);
  }
  else {
    rec_sel_by_view = mRecordListSelectedByViewList.Get();
  }

  // Now add the selected records to the view.
  // Note that the view might already have selected some records.
  unsigned count = 0;
  Record rec_tmp(AttributesTotal(), TRUE);
  //
  for (MacroListIterateAll(mTableRecordList)) {
    TableRecord* rec = mTableRecordList.Get();
    rec_tmp.mpTableRecord = rec;

    if (q->Match(&rec_tmp)) {
      Record* rec_new = new Record(rec);
      assert(rec_new);

      rec_sel_by_view->mpRecordList->Add(rec_new); // matched query
      count++;
    }
  }

  return select_count = count;
}

bool Table::RecordsUnselect(View* view, Record* rec_unselect)
{
  // As the view already has the record handle it means it was already imported.
  // We need to just remove it from the view's record list and decrement the
  // reference count.
  bool found = FALSE;

  List<Record>& rec_list = *view->mpRecordPtrList;
  MacroListDeleteLink(rec_list, rec_unselect, found);

  if (found) rec_unselect->mpTableRecord->ReferenceCountDecrement();

  return found;
}

bool Table::ViewEnable(View* view, bool is_owner)
{
  // The Enable operation mainly consists of checking for permissions.

  // First we need to find the view's permission.
  unsigned view_perm;

  bool found = FALSE;
  for (MacroListIterateAll(mViewInfoList)) {
    ViewInfo *view_perms = mViewInfoList.Get();

    if (view_perms && view_perms->mpView==view) {
      found     = TRUE;
      view_perm = view_perms->mViewRecPerm;

      delete view_perms;
      mViewInfoList.DeleteLinkThenIterationDone();

      break;
    }
  }

  if (!found) return FALSE;

  bool tbl_perm_ok = FALSE; // unless found ok below

  int mask_add    = eRecPerm_OwnerGroupOrOtherAdd;
  int mask_delete = eRecPerm_OwnerGroupOrOtherDelete;
  int mask_modify = eRecPerm_OwnerGroupOrOtherModify;
  int mask_read   = eRecPerm_OwnerGroupOrOtherRead;

  // 1. Check whether views record 'view_perm' violates 'mTableOpenFlags'.
  // Violations are:
  //   Table::!Append  View::(Owner,Group,Other)x(Add)
  //   Table::!Delete  View::(Owner,Group,Other)x(Delete)
  //   Table::!Modify  View::(Owner,Group,Other)x(Modify)
  //   Table::!Read    View::(Owner,Group,Other)x(Read)
  if ((view_perm & mask_add)    && (mTableOpenFlags & eTableOpenFlagBit_PermAppend)) tbl_perm_ok = TRUE;
  if ((view_perm & mask_delete) && (mTableOpenFlags & eTableOpenFlagBit_PermDelete)) tbl_perm_ok = TRUE;
  if ((view_perm & mask_modify) && (mTableOpenFlags & eTableOpenFlagBit_PermModify)) tbl_perm_ok = TRUE;
  if ((view_perm & mask_read)   && (mTableOpenFlags & eTableOpenFlagBit_PermRead  )) tbl_perm_ok = TRUE;

  // 2. Check whether views record permission violates masters view's permission (if present).
  // Violations are (if MasterView exists, otherwise there are no violations):
  //   View::Owner (any bit set)
  //   View::Group (view is in  Group) x (Group bits not subset of master view Group)
  //   View::Group (view is not Group) x (any master view Group bit set)
  //   View::Other (Other bits not subset of master view Other)

  bool view_perm_ok          = TRUE; // unless denied below
  bool install_owner         = FALSE;
  bool view_belongs_to_group = mViewListGroup.Exists(view);

  if (!mpViewOwner) {
    if (is_owner == FALSE) {
      // No owner view installed and this view doesnt want to be the owner
      // either but we must have an owner to proceed with permission checks.
      view_perm_ok = FALSE;
    }
    else {
      // No master view yet, for now we make the first view the master.
      install_owner = TRUE;
    }
  }
  else if (is_owner==TRUE && view!=mpViewOwner) {
    // Owner already insatalled and this view wants to be the owner.
    view_perm_ok = FALSE;
  }
  else {
    // Check views 'Owner' permissions (should not be set).
    if (view_perm & eRecPerm_OwnerAll) view_perm_ok = FALSE;

    // Check views 'Group' permissions.
    if (view_belongs_to_group) {
      //  Not allowed to set any additional 'Group' permissions.
      if ((view_perm | mViewOwnerRecPermission) & eRecPerm_GroupAll != mViewOwnerRecPermission & eRecPerm_GroupAll)
      {
        view_perm_ok = FALSE;
      }
    }
    else {
      //  Not allowed to set any 'Group' permissions.
      if (view_perm & eRecPerm_GroupAll) view_perm_ok = FALSE;

      // 3. Check views 'Other' permissions.
      // Not allowed to set any additional 'Other' permissions.
      if ((view_perm & eRecPerm_OtherAll) | mViewOwnerRecPermission != mViewOwnerRecPermission & eRecPerm_OtherAll)
      {
        view_perm_ok = FALSE;
      }
    }
  }

  bool perm_ok;
  if (view_perm_ok && tbl_perm_ok) perm_ok = TRUE;
  else                             perm_ok = FALSE;

  // Finally, install the owner if so flagged.
  if (install_owner == TRUE) {
    mpViewOwner              = view;
    mViewOwnerRecPermission  = view_perm;
  }

  return perm_ok;
}

bool Table::RecordsImportSelected(View* view)
{
  // Note: We do allow this function to be called repeatedly
  //       to keep on adding more selected records to a view.

  // First we need to find the view's permission.
  unsigned view_perm;

  bool found = FALSE;
  for (MacroListIterateAll(mViewInfoList)) {
    ViewInfo *view_perms = mViewInfoList.Get();

    if (view_perms && view_perms->mpView==view) {
      found     = TRUE;
      view_perm = view_perms->mViewRecPerm;
      break;
    }
  }

  if (!found) {
    // The view was not found in the Table's 'view create' list.
    return FALSE;
  }

  // Next, find the list of records the view has selected
  // (but not yet imported).
  RecordsSelectedByView* rec_sel_by_view;

  found = FALSE;
  for (MacroListIterateAll(mRecordListSelectedByViewList)) {
    rec_sel_by_view = mRecordListSelectedByViewList.Get();

    if (rec_sel_by_view) {
      if (rec_sel_by_view->mpView == view) { found = TRUE; }
      else                                 { continue;     }
    }
  }

  if (!found) {
    // No record list found to import.
    return FALSE;
  }

  if (rec_sel_by_view->mpRecordList->Size()==0) {
    // No records found in list to be imported, however,
    // there is no reason to treat this as an error.
    return TRUE;
  }

  // Note that the views 'view_perm' permissions have already been checked
  // during ViewCreate().
  // In order to import records the view needs permission for delete, modify
  // or read. The 'read' permission for import is obvious but 'delete' and
  // 'modify' also implicitly require 'read' permission because these operations
  // cannot be invoked if the view doesnt have a reference to the record.
  // Note that permission of 'add' cannot be used to import records.

  if (view_perm &
      (  eRecPermBit_OwnerDelete| eRecPermBit_OwnerModify| eRecPermBit_OwnerRead
       | eRecPermBit_GroupDelete| eRecPermBit_GroupModify| eRecPermBit_GroupRead
       | eRecPermBit_OtherDelete| eRecPermBit_OtherModify| eRecPermBit_OtherRead))
  {
    // Ok to import records, also increment reference count.
    List<Record> *rec_list = rec_sel_by_view->mpRecordList;
    assert(rec_list);

    for (MacroListIterateAll((*rec_list))) {
      Record* rec = rec_list->Get();
      assert(rec);

      rec->ViewSet(view);
      rec->mpTableRecord->ReferenceCountIncrement();
      view->mpRecordPtrList->Add(rec);

      view->AggregatesUpdate(eAggrUpdType_RecordAdd, rec);

      // Lastly, check trigger on aggregates after update.
      if (! mTriggerAggregateList.IsEmpty()) {
        for (MacroListIterateAll(mTriggerAggregateList)) {
          Trigger *trg = mTriggerAggregateList.Get();

          if (trg->TriggerStateGet() == eTriggerState_Active)
            if (trg->TriggerMatch(rec))
              gScheduler->TriggerSchedule(trg);
        }
      }
    }
    mRecordListSelectedByViewList.DeleteLinkThenIterationDone();
    delete rec_sel_by_view;
    delete rec_list;

    return TRUE;
  }
  else {
    // Cannot import selected records, maybe discard them and the list
    // holding them or dont do anything, let the view decide whether to
    // prune the list or clear it entirely.

    return FALSE;
  }
}

bool Table::RecordImportFromTrigger(Trigger *trigger)
{
  // First we need to find the view's permission.
  unsigned view_perm;

  bool found = FALSE;
  for (MacroListIterateAll(mViewInfoList)) {
    ViewInfo *view_perms = mViewInfoList.Get();

    if (view_perms && trigger->mpView == view_perms->mpView) {
      found     = TRUE;
      view_perm = view_perms->mViewRecPerm;
      break;
    }
  }

  if (!found) {
    // The view was not found in the Table's 'view create' list.
    return FALSE;
  }

  if (view_perm &
      (  eRecPermBit_OwnerDelete| eRecPermBit_OwnerModify| eRecPermBit_OwnerRead
       | eRecPermBit_GroupDelete| eRecPermBit_GroupModify| eRecPermBit_GroupRead
       | eRecPermBit_OtherDelete| eRecPermBit_OtherModify| eRecPermBit_OtherRead))
  {
    assert(trigger->mpRecord);

    trigger->mpRecord->ViewSet(mpViewOwner);
    trigger->mpRecord->mpTableRecord->ReferenceCountIncrement();
    trigger->mpView->mpRecordPtrList->Add(trigger->mpRecord);

    trigger->mpView->AggregatesUpdate(eAggrUpdType_RecordAdd, trigger->mpRecord);

    // Lastly, check trigger on aggregates after update.
    if (! mTriggerAggregateList.IsEmpty()) {
      for (MacroListIterateAll(mTriggerAggregateList)) {
        Trigger *trg = mTriggerAggregateList.Get();

        if (trg->TriggerStateGet() == eTriggerState_Active)
          if (trg->TriggerMatch(trigger->mpRecord))
            gScheduler->TriggerSchedule(trg);
      }
    }
  }
}

bool Table::RecordAdd(View* view, Record* rec)
{
  if (rec == 0) return FALSE;

  bool ret = FALSE;

  // Check for 'add' permission, if ok then mark it for adding to the table.
  if (IsOwnerView(view) && (mViewOwnerRecPermission & eRecPermBit_OwnerAdd)
  ||  IsGroupView(view) && (mViewOwnerRecPermission & eRecPermBit_GroupAdd)
  ||                       (mViewOwnerRecPermission & eRecPermBit_OtherAdd))
  {
    // In a table we dont care where the record gets added,
    // by default it will get added at the end.
    mTableRecordList.Add(rec->mpTableRecord);

    ret = TRUE;
  }
  else
    ret = FALSE;

  return ret;
}

bool Table::TranRecordAdd(View* view, Record *rec)
{
  if (rec == 0) return FALSE;

  bool ret = FALSE;

  // Check for 'add' permission, if ok then mark it for adding to the table.
  if (IsOwnerView(view) && (mViewOwnerRecPermission & eRecPermBit_OwnerAdd)
  ||  IsGroupView(view) && (mViewOwnerRecPermission & eRecPermBit_GroupAdd)
  ||                       (mViewOwnerRecPermission & eRecPermBit_OtherAdd))
  {
    mpDatabase->DBTransactionsAdd(eDBTransRecOper_RecordAdd, view, rec);

    ret = TRUE;
  }
  else
    ret = FALSE;

  return ret;
}

bool Table::TranRecordDelete(View* view, Record* rec)
{
  if (rec == 0) return FALSE;

  // To delete a record we will follow the following steps:
  // 1. Check for 'delete' permission, if ok then mark it for delete.
  // 2. Always decrement the records reference count unless it is at 0.
  // 3. If the reference count is 0 then we add the delete operation to
  //    the DBTransaction list where it will committed at the next commit cycle.
  bool ret;

  if (IsOwnerView(view) && (mViewOwnerRecPermission & eRecPermBit_OwnerDelete)
  ||  IsGroupView(view) && (mViewOwnerRecPermission & eRecPermBit_GroupDelete)
  ||                       (mViewOwnerRecPermission & eRecPermBit_OtherDelete))
  {
    ret = TRUE;

    mpDatabase->DBTransactionsAdd(eDBTransRecOper_RecordDelete, view, rec);
  }
  else {
    assert(0);
    ret = FALSE;
  }

  return ret;
}

bool Table::TranRecordModify(View* view, Record *rec, const Attribute* attr, const Value& val)
{
  if (rec == 0) return FALSE;

  bool ret = FALSE;

  // To modify a record we will follow the following steps:
  // 1. Check for 'modify' permission, if ok then proceed.

  if (IsOwnerView(view) && (mViewOwnerRecPermission & eRecPermBit_OwnerModify)
  ||  IsGroupView(view) && (mViewOwnerRecPermission & eRecPermBit_GroupModify)
  ||                       (mViewOwnerRecPermission & eRecPermBit_OtherModify))
  {
    mpDatabase->DBTransactionsAdd(eDBTransRecOper_RecordValueModify, view, rec, attr, val);

    ret = TRUE;
  }
  else
    assert(0);
}

const Attribute* Table::AttributeFind(const char* attr_name)
{
//for (MacroListIterateAll(mAttributeList)) {
  for (mAttributeList.First(); ! mAttributeList.IsDone(); mAttributeList.Next()) {
    Attribute* attr = mAttributeList.Get();

    if (attr && !strcmp(attr->Name(), attr_name)) {
      attr->mpTable = this; // TBD: Why is this needed? for Joins?
                            //      Yes, but better to use a private function.
      return attr;
    }
  }

  return 0;
}

const Attribute* Table::AttributeFind(short index)
{
  if (mAttributeList.PositionSet((unsigned)index)) {
    Attribute* attr = mAttributeList.Get();
    assert(attr);

  //attr->mpTable = this; // TBD: Why was this needed?
    return attr;
  }

  return 0;
}

short Table::AttributesTotal() const
{
  return mAttributeList.Size();
}

eValueType Table::AttributeType(short index)
{
  short count = 0;

  for (mAttributeList.First(); ! mAttributeList.IsDone(); mAttributeList.Next())
  {
    Attribute *attr = mAttributeList.Get();

    if (count == index) { assert(attr); return attr->mValueType; }
    else                count++;
  }

  return eValueUnknown;
}

Trigger* Table::TriggerNew(const char* trg_name, eTriggerType type, void* trg_owner_obj, TriggerActionFnPtr fn_ptr, View *v, Query *q, Record *rec)
{
  Trigger *trg = new Trigger(trg_name, type, trg_owner_obj, fn_ptr, v, q, rec); 
  assert(trg);

  bool is_added;

  switch (trg->TriggerType()) {
    case eTrgRecAdd:
      is_added = mTriggerRecordAddList.Add(trg);
      break;
    case eTrgRecDelete:
      is_added = mTriggerRecordDeleteList.Add(trg);
      break;
    case eTrgRecModify:
      is_added = mTriggerRecordModifyList.Add(trg);
      break;
    case eTrgAggregate:
      is_added = mTriggerAggregateList.Add(trg);
      break;
    default:
      assert(0);
  }

  assert (is_added);
}

bool Table::TriggerDelete(Trigger *trg)
{
  bool ret;

  switch (trg->TriggerType()) {
    case eTrgRecAdd:
      MacroListDeleteElm(mTriggerRecordAddList, trg, ret);
      break;
    case eTrgRecDelete:
      MacroListDeleteElm(mTriggerRecordDeleteList, trg, ret);
      break;
    case eTrgRecModify:
      MacroListDeleteElm(mTriggerRecordModifyList, trg, ret);
      break;
    case eTrgAggregate:
      MacroListDeleteElm(mTriggerAggregateList, trg, ret);
      break;
    default:
      assert(0);
  }

  if (!ret) {
    // Maybe it is still in the 'New' list (ie was not activated).
    MacroListDeleteElm(mTriggerNewList, trg, ret);
  }

  assert(ret);

  return ret;
}

//TBD:
//bool Table::TriggerMatched(Trigger *trg)
//{
//  return mpTable->TriggerDelete(trg);
//}
//
//bool Table::TriggerClear(Trigger *trg)
//{
//  return mpTable->TriggerDelete(trg);
//}

bool Table::Read(int flags)
{
  if (strlen(mpName) == 0) return FALSE;

  char  filename[MAX_FILENAME_LENGTH+1];
  short i;

  // TBD: Check flags to determine in what 'mode' to open the file and whether
  //      the operation is allowed.
  sprintf(filename, "%s/%s", gDefaultDatabaseDirectory, mpName);
  FILE* fp = fopen(filename, "r");

  if (fp == 0) return FALSE;

  // Initialize 'mAttributeList'
  char attr_name[MAX_ATTR_NAME_LENGTH+1];
  char attr_type[MAX_ATTR_TYPE_LENGTH+1];

  // TBD: Ensure safe scan by limiting length of string scanned.
  Attribute*  attr;
  eValueType  type;
  int         width;

  bool done   = FALSE;
  bool status = TRUE;

  while (!done && fscanf(fp, "%s %s", attr_name, attr_type) == 2) {

    type = AttributeStrToType(attr_type);
    attr = 0;

    if (type != eValueBit) {
      attr = new Attribute(attr_name, type);
      assert(attr);

      attr->mpTable = this;
    }
    else {
      if (fscanf(fp, "%d", &width) == 1) {
        attr = new Attribute(attr_name, eValueBit, (Value*)0, (short)width);
        assert(attr);

        attr->mpTable = this;
      }
      else { status=FALSE; done=TRUE; }
    }

    if (attr) mAttributeList.Add(attr);
  }

  if (status == FALSE) { 
    MacroListDeleteAll(mAttributeList, Attribute);

    fclose(fp);
    return FALSE;
  }

  short attr_total = AttributesTotal();

  // Make a 'template' record for all records read in.
  TableRecord tbl_rec_tmpl(attr_total); // use as template record

  // Keep local info for bit widths.
  short* bit_width = new short [attr_total]; M_UpdConstructCountN(eType_Short, attr_total);
  assert(bit_width);

  i = 0;
  for (MacroListIterateAll(mAttributeList)) {
    Attribute* attr = mAttributeList.Get();
    assert(attr);

    Value val(attr->ValueType()); M_Constructed(Value);

    tbl_rec_tmpl.mpValues[i] = val; // this assignment will set the type

    if (attr->ValueType() == eValueBit) bit_width[i] = attr->BitWidth();
    else                                bit_width[i] = 0;

    i++;
  }

  // Read in all the Records from the file.
  char  buf[MAX_RECORD_LINE_LENGTH+1];
  char* attr_ptr;

  short record_count = 0;
  short attr_sep_len = strlen(gAttributeSeparator);

  // fgets reads in at most n-1 characters
  for (status=TRUE, attr_ptr=buf;
   fgets(buf, MAX_RECORD_LINE_LENGTH+1, fp)!=NULL && status;
    record_count++)
  {
    char* last_char_ptr = buf + strlen(buf);
    char* attr_ptr      = buf;

    for (short i=0; i<attr_total; i++) {
      char* attr_sep_ptr = strstr(attr_ptr, gAttributeSeparator);

      if (attr_sep_ptr) {
        *attr_sep_ptr = '\0';

        if (! tbl_rec_tmpl.mpValues[i].Initialize(attr_ptr, bit_width[i]))
          status = FALSE;

        attr_ptr = attr_sep_ptr + attr_sep_len;

        if (attr_ptr > last_char_ptr) status = FALSE;
      }
      else
        status = FALSE;
    }

    if (status) mTableRecordList.Add(new TableRecord(tbl_rec_tmpl, attr_total));
  }

  if (bit_width) { delete [] bit_width; bit_width=0; M_UpdDestructCountN(eType_Short, attr_total); }

  fclose(fp);

  return status;
}

bool Table::Save(const char* file_name)
{
  short i;
  char  filename [MAX_FILENAME_LENGTH+1];

  filename[0] = '\0';
  strcat(filename, gDefaultDatabaseDirectory);

  if (file_name != 0) {
    assert((strlen(gDefaultDatabaseDirectory) + strlen(file_name)) <= MAX_FILENAME_LENGTH);

    strcat(filename, file_name);
  }
  else {
    assert((strlen(gDefaultDatabaseDirectory) + strlen(mpName)) <= MAX_FILENAME_LENGTH);

    // Use the name of the table by default.
    strcat(filename, mpName);
  }

  FILE* fp = fopen(filename, "w");

  assert(fp);

  bool status = TRUE;

  if (fp) {
    // Save Attributes
    for (MacroListIterateAll(mAttributeList)) {
      Attribute *attr = mAttributeList.Get();

      fprintf(fp, "%s %s ", attr->Name(), AttributeTypeToStr(attr->ValueType()));

      if (attr->ValueType() == eValueBit) fprintf(fp, "%d ", attr->BitWidth());
    }
    fprintf(fp, "%s", gRecordSeparator);

    // Save Records
    char           buf[MAX_RECORD_LINE_LENGTH+1];
    short          total_attr = AttributesTotal();
    TableRecord   *rec;

    for (MacroListIterateAll(mTableRecordList)) {
      rec = mTableRecordList.Get();

      for (short i=0; i<total_attr; i++) {
        fprintf(fp, "%s%s", rec->mpValues[i].Save(buf), gRecordSeparator);
      }

      fprintf(fp, "%s%s", buf, gRecordSeparator);
    }

    fclose(fp);
  }
  else {
    status = FALSE;
  }

  return status;
}

void Table::Print(char* attr_name)
{
  short i = SHRT_MAX;
  short attr_index;

  // Print Attributes.
  for (MacroListIterateAll(mAttributeList)) {
    Attribute *attr = mAttributeList.Get();

    printf("%s %s ", attr->Name(), AttributeTypeToStr(attr->ValueType()));

    if (attr->ValueType() == eValueBit) printf("%d ", attr->BitWidth());

    if (attr_name) if (!strcmp(attr_name, attr->Name())) attr_index = i;
  }
  printf("%s", gRecordSeparator);

  // Print Records
  char           buf[MAX_RECORD_LINE_LENGTH+1];
  short          total_attr = AttributesTotal();
  TableRecord   *rec;

  for (MacroListIterateAll(mTableRecordList)) {
    rec = mTableRecordList.Get();

    if (attr_index == SHRT_MAX) {
      rec->mpValues[attr_index].Save(buf);
      printf("%s", buf);
    }
    else {
      for (short i=0; i<total_attr; i++) {
        rec->mpValues[i].Save(buf);
        printf("%s ", buf);
      }
    }

    printf("%s", gRecordSeparator);
  }
}

bool Table::AggregatesUpdate(View* view_upd, eAggregateUpdateReasonType rsn_type, Record *rec_upd, const Attribute* attr_mdfy, Value *old_val)
{
  assert (rsn_type == eAggrUpdType_RecordModify);

  // Algo: if (record reference count is > 1)
  //         for (all views in the table other than current view)
  //           if (view has aggregates)
  //             if (view has the same table-record)
  //               update views aggregate;
  //         // done with views when all shared records are found
  if (rec_upd->mpTableRecord->mReferenceCount > 1) {
    short found_count = 0;
    for (MacroListIterateAll(mViewInfoList)) {
      ViewInfo *view_info = mViewInfoList.Get();
      assert(view_info);

      View* view = view_info->mpView;
      if (view != view_upd) {
        if (view->mAggregatePtrList.Size()) {
          for (MacroListIterateAll(*view->mpRecordPtrList)) {
            Record* rec = view->mpRecordPtrList->Get();
            assert(rec);

            if (rec_upd->mpTableRecord == rec->mpTableRecord) {
              view->AggregatesUpdate(eAggrUpdType_RecordModifyPropagate, rec, attr_mdfy, old_val);
              found_count++;
              break;
            }
          }
        }
      }

      if (rec_upd->mpTableRecord->mReferenceCount == found_count) break;
    }
  }
}

void Table::PrintAttributeNames()
{
  FILE* fp = stdout;

  for (MacroListIterateAll(mAttributeList)) {
    Attribute *attr = mAttributeList.Get();
    assert(attr);

    fprintf(fp, "%s ", attr->Name());
  }
  fprintf(fp, "\n");

  for (MacroListIterateAll(mAttributeList)) {
    Attribute *attr = mAttributeList.Get();
    assert(attr);

    fprintf(fp, "%s", AttributeTypeToStr(attr->ValueType()));
    if (attr->ValueType() == eValueBit) fprintf(fp, "(%d) ", attr->BitWidth());
    else                                fprintf(fp, " ");
  }
  fprintf(fp, "\n");
}

// End
