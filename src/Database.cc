// $RCSfile: Database.cc,v $
// $Revision: 1.6 $

#include <assert.h>
#include <string.h>

#include "Database.h"
//
#include "Attribute.h"
#include "DBTransaction.h"
#include "DBTransactionRec.h"
#include "DBTransactionRecValue.h"
#include "Global.h"
#include "Record.h"
#include "Scheduler.h"
#include "Table.h"
#include "TableRecord.h"
#include "Trigger.h"

extern Scheduler* gScheduler;

M_ExternConstructDestructCountInt;

Database::Database(const char* path_to_db, const char* db_name) :
  mpName       (db_name),
  mpPathToDB   (path_to_db)
{
  M_UpdConstructCount(eType_Database);
}

Database::~Database()
{
  if (mDBTransactionList.Size()) {
    assert(0);
  }

  if (mTableList.Size()) {
    printf("Not all tables cleaned up\n");
//  assert(0); // change to warning later, better to explicitly close all tables

    mTableList.DeleteLinkAll();
  }

  M_UpdDestructCount(eType_Database);
}

Table* Database::TableOpen(const char* table_name, int flags)
{
  // TBD: Database check table file lock or put the lock.

  bool name_exists = FALSE;

  for (MacroListIterateAll(mTableList))
  {
    Table *tbl = mTableList.Get();

    if (tbl && (!strcmp(tbl->Name(), table_name))) {
      name_exists = TRUE;
      break;
    }
  }

  Table* new_tbl = 0;

  if (! name_exists) {
    new_tbl = new Table(this, table_name, flags);

    if (new_tbl != 0) mTableList.Add(new_tbl);
    else              assert(0);
  }
  else {
    assert(0);
  }

  return new_tbl;
}

bool Database::TableClose(Table *tbl)
{
  bool ret;
  MacroListDeleteElm(mTableList, tbl, ret);
  assert(ret);
}

void Database::DBTransactionsAdd(eDBTransactionOperation oper, View *view, Record *rec)
{
  switch (oper) {
    case eDBTransRecOper_RecordAdd:
    case eDBTransRecOper_RecordDelete:
    {
      DBTransactionRec *tran_rec = new DBTransactionRec(oper, view, rec);

      if (tran_rec) mDBTransactionList.Add((DBTransaction*)tran_rec);
      else          assert(0);

      break;
    }
    default: assert(0);
  }
}

void Database::DBTransactionsAdd(eDBTransactionOperation oper, View *view, Record* rec, const Attribute* attr, const Value& val)
{
  switch (oper) {
    case eDBTransRecOper_RecordValueModify: {
      DBTransactionRecValue *tran_rec_val = new DBTransactionRecValue(oper, view, rec, attr, val);

      if (tran_rec_val) mDBTransactionList.Add((DBTransaction*)tran_rec_val);
      else              assert(0);

      break;
    }
    default: assert(0);
  }
}

short Database::DBTransactionsCommitView(View* view)
{
  bool     ret;
  bool     del_elm;
  short    count;
  
  count = 0;

  for (M_ListIterateAndDelete(mDBTransactionList, del_elm=TRUE))
  {
    // The 'commit' steps are:
    // 1. Check if commit is invoked for a specific view?
    // 2. Perform the actual operation.
    // 3. Update any 'aggregates'.
    // 4. Examine trigger conditions on records and invoke actions if true.
    // 5. Lastly, invoke triggers on aggregates.
    // The exception is 'Delete', here the actual operation is performed last
    // because we need the record until aggregate and trigger logic is done.

    // TBD: Remember to perform the 'commit' in a safe order. For example first
    //      do record transactions, then table transactions so that table is not
    //      deleted before any of its records are committed.
    //      Currently we have not implemented transaction for table delete so no
    //      special handling is needed yet.

    DBTransaction *db_tran = mDBTransactionList.Get();
    assert(db_tran);

    switch (db_tran->mTransOper) {
      case eDBTransRecOper_RecordAdd: {

        DBTransactionRec* db_tran_rec = (DBTransactionRec*)db_tran;

        // Check if 'add' commit is invoked for a specific view?
        if (view && db_tran_rec->mpView!=view) continue;

        // Perform the commit operation.
      //db_tran_rec->mpView->mpTable->mTableRecordList.Add(db_tran_rec->mpRecord->mpTableRecord);
        db_tran_rec->mpView->mpTable->RecordAdd(view, db_tran_rec->mpRecord);
        db_tran_rec->mpRecord->mpView = view;
        db_tran_rec->mpView->mpRecordPtrList->Add(db_tran_rec->mpRecord);

        count++;

        // Update Aggregates:
        // Update for Aggregate is possible for db_tran_rec->mpView
        // and is postphoned for other views that will add this record via
        // triggers.
        // TBD: Maybe aggregate update needs to be scheduled too?
        if (db_tran_rec->mpView->mAggregatePtrList.Size()) // add fn like HasAggregates()
          db_tran_rec->mpView->AggregatesUpdate(eAggrUpdType_RecordAdd, db_tran_rec->mpRecord);
        // Check 'Add' for Triggers.
        List<Trigger> &trg_list = db_tran_rec->mpView->mpTable->mTriggerRecordAddList;
        if (! trg_list.IsEmpty()) {
          for (MacroListIterateAll(trg_list)) {
            Trigger *trg = trg_list.Get();

            if (trg->TriggerStateGet() == eTriggerState_Active) {
              if (trg->TriggerMatch(db_tran_rec->mpRecord)) {
                trg->TriggerRecordSet(db_tran_rec->mpRecord);
                gScheduler->TriggerSchedule(trg);

                if (trg->TriggerStateGet() == eTriggerState_RemoveTrigger) {
                  trg_list.DeleteLinkThenIterationDone();
                }
              }
            }
          }
        }

        delete db_tran_rec;

        // Check for 'Add' Triggers on Aggregates:
        // Cannot accomplish this here (see 2 above).
        // The current mechanism is that only when the record is imported into 
        // the view will the table class update the aggregates and evaluate the
        // aggregate triggers.

        break;
      }
      // Done case eDBTransRecOper_RecordAdd.

      case eDBTransRecOper_RecordDelete: {
        DBTransactionRec *db_tran_rec = (DBTransactionRec*)db_tran;
        View             *tran_view   = db_tran_rec->mpView;
        Record           *tran_rec    = db_tran_rec->mpRecord;

        // Check if 'delete' commit is invoked for a specific view?
        if (view && tran_view!=view) continue;

        // Update Aggregate:
        if (tran_view->mAggregatePtrList.Size()) // maybe use fn like HasAggregates()
          tran_view->AggregatesUpdate(eAggrUpdType_RecordDelete, tran_rec);

        // Check for 'Delete' Triggers.
        List<Trigger> &trg_list = db_tran_rec->mpView->mpTable->mTriggerRecordDeleteList;
        if (! trg_list.IsEmpty()) {
          for (MacroListIterateAll(trg_list)) {
            Trigger *trg = trg_list.Get();

            if (trg->TriggerStateGet() == eTriggerState_Active) {
              if (trg->TriggerMatch(db_tran_rec->mpRecord)) {
                // TBD: Determine if trg->mView contains mpRecord before proceeding.
                //      Note, if reference count is 1 then only db_tran_rec->mpView has the record.
                // TBD: Create and supply a copy of the record (not the orig).
                trg->TriggerRecordSet(db_tran_rec->mpRecord);
                gScheduler->TriggerSchedule(trg);

                if (trg->TriggerStateGet() == eTriggerState_RemoveTrigger) {
                  trg_list.DeleteLinkThenIterationDone();
                }
              }
            }
          }
        }

        // Lastly, perform the commit 'delete' operation.

        TableRecord* table_record = tran_rec->mpTableRecord;

        delete tran_rec;

        // If there are no more references to this record
        // then we can delete it from the view's table.
        if (table_record->ReferenceCountGet() == 0) {
          List<TableRecord> &tbl_rec_list = db_tran_rec->mpView->mpTable->mTableRecordList;
          MacroListDeleteElm(tbl_rec_list, table_record, ret);
          assert(ret);
        }

        List<Record>& rec_list = *tran_view->mpRecordPtrList;
        MacroListDeleteElm(rec_list, tran_rec, ret);
        assert(ret);

        count++;

        delete db_tran_rec;

        break;
      }
      // Done case eDBTransRecOper_RecordDelete.

      case eDBTransRecOper_RecordValueModify: {
        DBTransactionRecValue *db_tran_rec_val = (DBTransactionRecValue*)db_tran;

        // Check if 'modify' commit is invoked for a specific view?
        if (view && db_tran_rec_val->mpView != view) continue;

        const Attribute*    attr    = db_tran_rec_val->mpAttribute;
        Record*             rec     = db_tran_rec_val->mpRecord;
        Value*              new_val = db_tran_rec_val->mpValue; M_Constructed(Value);

        // OPT: Check if possible to avoid making copy of current (old) value.
        Value old_val; M_Constructed(Value);

        // First perform the commit operation.
        db_tran_rec_val->mpRecord->ValueCopyGet(attr, old_val);
        db_tran_rec_val->mpRecord->ValueSet(attr, *new_val);
        count++;

        // Update Aggregate:
        if (db_tran_rec_val->mpView->mAggregatePtrList.Size())
          db_tran_rec_val->mpView->AggregatesUpdate(eAggrUpdType_RecordModify, rec, attr, &old_val);

        // Check for 'Modify' Triggers.
        List<Trigger> &trg_list = db_tran_rec_val->mpView->mpTable->mTriggerRecordModifyList;
        if (! trg_list.IsEmpty()) {
          for (MacroListIterateAll(trg_list)) {
            Trigger *trg = trg_list.Get();

            if (trg->TriggerStateGet() == eTriggerState_Active) {
              if (trg->TriggerMatch(db_tran_rec_val->mpRecord)) {
                // TBD: Determine if trg->mView contains mpRecord before proceeding.
                //      Note, if reference count is 1 then only db_tran_rec->mpView has the record.
                trg->TriggerRecordSet(db_tran_rec_val->mpRecord);
                gScheduler->TriggerSchedule(trg);

                if (trg->TriggerStateGet() == eTriggerState_RemoveTrigger) {
                  trg_list.DeleteLinkThenIterationDone();
                }
              }
            }
          }
        }

        delete db_tran_rec_val;

        break;
      }
      // Done case eDBTransRecOper_RecordValueModify.
    }
  }

  return count;
}

// End
