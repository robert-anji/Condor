// $RCSfile: Table.h,v $
// $Revision: 1.10 $

#ifndef TableClass
#define TableClass

#include "List.h"
#include "Database.h"
#include "DBTables.h"
#include "Value.h"    // needed for eValueType

class Aggregate;
class Attribute;
class Query;
class Process;
class Record;
class TableRecord;
class Trigger;
class View;

// TBD: Move these structs to Table::private ?
struct RecordsSelectedByView {
  const View*        mpView;
  List<Record>*      mpRecordList;
};
//
struct ViewInfo {
  ViewInfo(View* view, unsigned perm) : mpView(view), mViewRecPerm(perm) {}

  View*       mpView;
  unsigned    mViewRecPerm;
};

// Remove these?
#define MacroTableAttributesStart(TableName) TableAttributes Attributes##TableName[]
#define MacroTableAttributes(TableName)      Attributes##TableName
#define MacroTableAttributesEnd              { "",  eValueUnknown }

class Table
{
  friend class Database;

public:
  // Constructors/Destructor:
  // These are private, access is only by friend class Database.

  // Add attributes, each table should have at least one attribute.
  Attribute* AttributeAdd  (const char* attr_name, eValueType val_type, Value *null_val=0, short width=0);
  bool       AttributesAdd (TableAttributes* attr);

  // Utilities:
  bool        Save  (const char* filename=0);
  void        Print (char* attrib_name=0);
  const char* Name  () const { return mpName; }

  // The remaining functions are used from classes like Database and View,
  // and the user should not normally invoke these directly.

  // The most (and maybe only) important feature of a table
  // is the concept and the usage of Views. These are defined
  // in the functions below (all have a prefix of 'View').
  //
  View*    ViewCreate    (const char* view_name, unsigned perm);
  bool     ViewEnable    (View* view, bool is_owner);
  bool     ViewDelete    (View* view);
  bool     ViewGroupAdd  (View* view_for_group, View* owner_view);
  View*    ViewCreateCopy(const char* name, View* view) { return view; } // TBD

  // Views can select records, obtain ownership/permission, then release them.
  // The 'select' & 'permission' operations are kept separate for flexibility.
  unsigned RecordsSelect     (const View* v, unsigned start, unsigned end);
  unsigned RecordsSelect     (const View* v, Query* q);
  // TBD: (next line)\
  unsigned RecordsSelect     (const View* v, Regex* r); // upcoming feature
  bool     RecordsUnselect   (View* v, Record* rec);
  //
  bool     RecordsImportSelected    (View *v);
  bool     RecordImportFromTrigger  (Trigger *trigger);
  //
  bool     RecordAdd   (View* v, Record *rec);
//bool     RecordDelete(View* v, Record *rec);
//bool     RecordModify(View* v, Record *rec, const Attribute* attr, const Value &v);
  //
  bool     TranRecordAdd   (View* v, Record *rec);
  bool     TranRecordDelete(View* v, Record *rec);
  bool     TranRecordModify(View* v, Record *rec, const Attribute* attr, const Value &val);

  const char* ValueSeparatorGet() { return ">|<"; }

  // Attribute information.
  const Attribute*   AttributeFind    (const char* attr_name);
  const Attribute*   AttributeFind    (short index);
  short              AttributesTotal  () const;
  eValueType         AttributeType    (short position);

  // Aggregate update needed when a record is shared by multiple views.
  bool AggregatesUpdate(View* view, eAggregateUpdateReasonType rsn_type, Record *rec, const Attribute* mdfy_attr, Value *old_val);

  // Triggers for Table records.
  Trigger* TriggerNew(const char* trg_name, eTriggerType type, void *trg_owner_obj, TriggerActionFnPtr fn_ptr, View *v, Query *q, Record *rec);
  // Triggers for Table aggregates.
  Trigger* TriggerNew(eTriggerType type, void *trg_owner_obj, TriggerActionFnPtr fn_ptr, View *v, Aggregate *aggr, Query *q);
  //
  bool     TriggerActivate (Trigger* trg, TriggerActionFnPtr fn);
  bool     TriggerActivate (Trigger* trg, Process* process);
  bool     TriggerDelete   (Trigger *trg);

  // Meta attributes that each table has.
  Attribute* mpAttr_RecordPtr;
  Attribute* mpAttr_RecordCount;

  void PrintAttributeNames();
private:
  // Data
  const char*            mpName;
  Database*              mpDatabase;
  int                    mTableOpenFlags;
  eTableCommitMode       mTableCommitMode;
  //
  List<Attribute>        mAttributeList;
  List<TableRecord>      mTableRecordList;
  View*                  mpViewOwner; // only one owner, no list needed
  List<View>             mViewListGroup;
  unsigned               mViewOwnerRecPermission;
  List<Trigger>          mTriggerNewList;
  List<Trigger>          mTriggerRecordAddList;
  List<Trigger>          mTriggerRecordDeleteList;
  List<Trigger>          mTriggerRecordModifyList;
  List<Trigger>          mTriggerAggregateList;

  List<RecordsSelectedByView> mRecordListSelectedByViewList;
  List<ViewInfo>              mViewInfoList;

  // Functions:
  Table(Database* db, const char* tbname, int flags=eTableOpenFlags_Default);
  Table(Table& table);
  ~Table();
  //
  bool     RecordsImportCheckPermission(View* view, eRecordPermissionBits perm);
  //
  bool     Read(int flags);
  //
  inline bool     IsOwnerView(const View* v) { if (mpViewOwner==v) return TRUE;
                                               else                return FALSE; }
  inline bool     IsGroupView(View* v) { if (mViewListGroup.Exists(v)) return TRUE;
                                         else                          return FALSE; }
};

#endif

// End
