// $RCSfile: Record.h,v $
// $Revision: 1.7 $

#ifndef RecordClass
#define RecordClass

#include <stdio.h>

#include "DBDefines.h"
#include "List.h"
#include "TableRecord.h"
#include "Value.h"
#include "View.h"

class Attribute;
class DBTransaction;
class Condition;
class Query;
//class TableRecord;

class Record
{
//------------------------------------------------------------------------------
  friend class Scheduler; // for debug only, remove later
  friend class Database;
  friend class Table;
  friend class View;
//------------------------------------------------------------------------------
public:
  // Constructors are private, only View should create/destruct them.

  Record& operator[](const Attribute* attr);
  Record& Set(unsigned u);

  Record&  operator= (const Record& rec);
  bool     operator==(const Record& rec) const;
  bool     operator!=(const Record& rec) const;

  const Value& operator[](int index) { return mpTableRecord->mpValues[index]; }
  const Value* ValueGet(const Attribute* attr);

  // Comparison operators.
  // Record:
  bool     operator==(Record& rec);
  bool     operator!=(Record& rec);
  bool     operator>=(Record& rec);
  bool     operator> (Record& rec);
  bool     operator<=(Record& rec);
  bool     operator< (Record& rec);
  // Value:
  bool     operator==(const Value& val);
  bool     operator!=(const Value& val);
  bool     operator>=(const Value& val);
  bool     operator> (const Value& val);
  bool     operator<=(const Value& val);
  bool     operator< (const Value& val);
  bool     operator+=(const Value& val);
  bool     operator-=(const Value& val);
  // unsigned:
  bool     operator==(unsigned u);
  bool     operator!=(unsigned u);
  bool     operator>=(unsigned u);
  bool     operator> (unsigned u);
  bool     operator<=(unsigned u);
  bool     operator< (unsigned u);
  bool     operator+=(unsigned u);
  bool     operator-=(unsigned u);
  // int:
  bool     operator==(int i);
  bool     operator!=(int i);
  bool     operator>=(int i);
  bool     operator> (int i);
  bool     operator<=(int i);
  bool     operator< (int i);
  bool     operator+=(int i);
  bool     operator-=(int i);
  // char*:
  bool     operator==(char* s);
  bool     operator!=(char* s);
  bool     operator>=(char* s);
  bool     operator> (char* s);
  bool     operator<=(char* s);
  bool     operator< (char* s);
  // const char*:
  bool     operator==(const char* s);
  bool     operator!=(const char* s);
  bool     operator>=(const char* s);
  bool     operator> (const char* s);
  bool     operator<=(const char* s);
  bool     operator< (const char* s);
  // Bit
  bool     operator==(const Bit& b);
  bool     operator!=(const Bit& b);
  bool     operator>=(const Bit& b);
  bool     operator> (const Bit& b);
  bool     operator<=(const Bit& b);
  bool     operator< (const Bit& b);
  // void*:
  bool     operator==(void* ptr);
  bool     operator!=(void* ptr);
  // bool:
  bool     operator==(bool b);
  bool     operator!=(bool b);

  bool     operator! ();

  // Value assignment operator, if the record belongs
  // to a view this will cause a database transaction.
  bool     operator=(const Value&   val);
  bool     operator=(const char*    val);
  bool     operator=(void*          val);
  bool     operator=(unsigned       val);
  bool     operator=(Bit&           bit);

  // Compare() used by List<> to sort
  eCompare Compare(Record& rec);

  // Retrieve Value 'by value'. This should be ok because a client can only
  // have the record reference if it has read permission which is sufficient
  // to permit retrieving 'by value'.
  void     ValueCopyGet(const Attribute* attr, Value &val);
  void     ValueCopyGet(short pos, Value &val);
  void     ValueCopyGet(const Attribute* attr, Bit &bit);
  void     ValueCopyGet(short pos, Bit &bit);
  void*    GetValueAsPointer(const Attribute* attr);
  unsigned GetValueAsUnsigned(const Attribute* attr);
  // TBD: Also allow retrieving short, int, char etc.

  // Cast functions. // TBD: Find out why enabling this affects the Interface compile
//operator unsigned();

  // Initialize the View
  bool     ViewSet(View* v);

  // Needed to support Bit comparisons.
  void SetMsbLsb(short msb, short lsb);

  void Print(FILE* fp=stdout);
  void Print(short total, FILE* fp); // use when Record doesnt have a view yet
  void p();

  View*              mpView;        // make public for dbg
//------------------------------------------------------------------------------
private:
  // Data:
  TableRecord*       mpTableRecord; // pointer to TableRecord for actual Record values
//View*              mpView;        // view the record belongs to, moved to public for now
  //
  const Attribute*   mpAttribute;   // identify which record attribute to operate on
  bool               mAllowValUpdBeforeCommit;

  // For debug:
  unsigned mRecId;

  // Functions:
  Record(short attr_total, bool auto_commit);
  Record(TableRecord* tbl_rec);
  ~Record();

  bool     ValueSet(const Attribute* attr, Value &val); // TBD: remove after change in Database
//------------------------------------------------------------------------------
};

#endif

// End
