// $RCSfile: TableRecord.cc,v $
// $Revision: 1.5 $

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "TableRecord.h"
//
#include "DBDefines.h"
#include "Global.h"

static bool gDbg = FALSE;

M_ExternConstructDestructCountInt;

TableRecord::TableRecord(short total_attr) :
  mpValues             (0),
  mReferenceCount      (1)
{
  if (total_attr)
    mpValues = new Value [total_attr]; M_ConstructedN(Value, total_attr);

  M_UpdConstructCount(eType_TableRecord);
}

TableRecord::TableRecord(const TableRecord &rec, short total_attr) :
  mpValues             (0),
  mReferenceCount      (1)
{
  if (total_attr) {
    mpValues = new Value [total_attr]; M_ConstructedN(Value, total_attr);

    for (short i=0; i<total_attr; i++) {
      mpValues[i] = rec.mpValues[i];
    }
  }

  M_UpdConstructCount(eType_TableRecord);
}

TableRecord::~TableRecord()
{
  if (mpValues) {
    delete [] mpValues; mpValues = 0;
  }

  M_UpdDestructCount(eType_TableRecord);
}

void TableRecord::ValueTypeSet(short i, eValueType type)
{
  mpValues[i].ValueTypeSet(type);
}

short TableRecord::ReferenceCountGet() const
{
  return mReferenceCount;
}

short TableRecord::ReferenceCountIncrement()
{
  if (mReferenceCount != SHRT_MAX) mReferenceCount++;
  else                             assert(0);

  return mReferenceCount;
}

short TableRecord::ReferenceCountDecrement()
{
  if (mReferenceCount != 0) mReferenceCount--;
  else                      assert(0);

  return mReferenceCount;
}

// End
