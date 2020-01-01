// $RCSfile: TableRecord.h,v $
// $Revision: 1.2 $

#ifndef TableRecordClass
#define TableRecordClass

#include "Value.h"

class TableRecord
{
  friend class Database;
  friend class Table;
  friend class Record;

public:
  void ValueTypeSet(short i, eValueType type);

private:
  // Data:
  Value*   mpValues;          // pointer array of all Record Values
  short    mReferenceCount;   // Count of views that refer this Record

  // Functions:
  TableRecord(short total_attr);
  TableRecord(const TableRecord &rec, short total_attr);
  ~TableRecord();
  //
  // Get/Update the reference count
  short ReferenceCountGet() const;
  short ReferenceCountIncrement();
  short ReferenceCountDecrement();
};

#endif

// End
