// $RCSfile: Record.cc,v $
// $Revision: 1.8 $

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Record.h"
//
#include "Attribute.h"
#include "Bit.h"
#include "BucketList.h"
#include "Condition.h"
#include "DBDefines.h"
#include "Enum.h"
#include "Global.h"
#include "Query.h"
#include "TableRecord.h"
#include "Value.h"

extern int gDbg;

static unsigned sRecordId;

enum eAttrPosition { eAttrPositionInvalid = SHRT_MAX };

extern NameVal OperatorNV[];
M_ExternConstructDestructCountInt;

Record::Record(short total_attr, bool auto_commit) :
  mpView                   (0),
  mpAttribute              (0),
  mpTableRecord            (0),
  mAllowValUpdBeforeCommit (auto_commit),
  mRecId                   (sRecordId++)
{
  mpTableRecord = new TableRecord(total_attr);
  printf("MEM: mpTableRecord=%p\n", (void*)mpTableRecord);

  assert(mpTableRecord);

  M_UpdConstructCount(eType_Record);
}

Record::Record(TableRecord* tbl_rec) :
  mpView                   (0),
  mpAttribute              (0),
  mpTableRecord            (tbl_rec),
  mAllowValUpdBeforeCommit (TRUE),
  mRecId                   (sRecordId++)
{
  assert(mpTableRecord);

  mpTableRecord->ReferenceCountIncrement();

  M_UpdConstructCount(eType_Record);
}

Record::~Record()
{
  mpTableRecord->ReferenceCountDecrement();

  M_UpdDestructCount(eType_Record);
}

bool Record::ValueSet(const Attribute* attr, Value &val)
{
  short p = attr->PositionGet();

  // Value assignment will check for legality of assignment.

  if (p == eMaxAttributes) return FALSE;

  mpTableRecord->mpValues[p] = val;

  return TRUE;
}

const Value* Record::ValueGet(const Attribute* attr)
{
  return &(mpTableRecord->mpValues[attr->PositionGet()]);
}

Record& Record::operator[](const Attribute* attr)
{
  assert(attr);

  mpAttribute = attr;

  return *this;
}

Record& Record::Set(unsigned u)
{
  mpTableRecord->mpValues[mpAttribute->PositionGet()] = u;
  
  return *this;
}

// TBD: Revisit this once we have sorted records logic back again.
//eCompare Record::Compare(Record& rec)
//{
//  short pos = mpView->sortAttributePos;
//
//  // If the sort pos is invalid return eCompareEqual
//  if (pos == eMaxAttributes) {
//    assert(0);
//    return eCompareNotEqual;
//  }
//
//  return (mpValues[pos].Compare(rec.mpValues[pos]));
//}

Record& Record::operator=(const Record& rec)
{
  // We only allow Record assignment if the records belong to the same view
  // or if the record doesnt yet have a view (happens for new records).
  assert(mpView==0 || mpView==rec.mpView);

//if (mpView == 0) mpView = rec.mpView;
  short total;
  if (mpView == 0) total = rec.mpView->AttributesTotal();
  else             total =     mpView->AttributesTotal();

  short i;

  // TBD: Maybe change 'record[attr] = value;' to not use a db transaction.
  for (i=0; i<total; i++) {
  //const Attribute* attr = mpView->AttributeFind(i);
  //(*this)[attr]         = rec.mpTableRecord->mpValues[i];
    this->mpTableRecord->mpValues[i] = rec.mpTableRecord->mpValues[i];
  }

  return *this;
}

bool Record::operator==(const Record& rec) const
{
  short total = mpView->AttributesTotal();

  for (short i=0; i<total; i++)
  {
    if (mpTableRecord->mpValues[i] != rec.mpTableRecord->mpValues[i])
      return FALSE;
  }

  return TRUE;
}

bool Record::operator!=(const Record& rec) const
{
  if (*this == rec) return FALSE;
  else              return TRUE;
}

bool Record::operator==(Record& rec)
{
  if (this == &rec) return TRUE;

  bool result = (*this == rec.mpTableRecord->mpValues[rec.mpAttribute->PositionGet()]);

  rec.mpAttribute = 0;

  return result;
}

bool Record::operator!=(Record& rec)
{
  if (this == &rec) return FALSE;

  bool result = (*this != rec.mpTableRecord->mpValues[rec.mpAttribute->PositionGet()]);

  rec.mpAttribute = 0;

  return result;
}

bool Record::operator>=(Record& rec)
{
  if (this == &rec) return TRUE;

  bool result = (*this >= rec.mpTableRecord->mpValues[rec.mpAttribute->PositionGet()]);

  rec.mpAttribute = 0;

  return result;
}

bool Record::operator>(Record& rec)
{
  if (this == &rec) return TRUE;

  bool result = (*this > rec.mpTableRecord->mpValues[rec.mpAttribute->PositionGet()]);

  rec.mpAttribute = 0;

  return result;
}

bool Record::operator<=(Record& rec)
{
  if (this == &rec) return TRUE;

  bool result = (*this <= rec.mpTableRecord->mpValues[rec.mpAttribute->PositionGet()]);

  rec.mpAttribute = 0;

  return result;
}

bool Record::operator<(Record& rec)
{
  if (this == &rec) return TRUE;

  bool result = (*this < rec.mpTableRecord->mpValues[rec.mpAttribute->PositionGet()]);

  rec.mpAttribute = 0;

  return result;
}

bool Record::operator==(const Value& val)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == val);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(const Value& val)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] != val);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>=(const Value& val)
{
  bool compare;

  assert(mpAttribute);
  
  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] >= val);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>(const Value& val)
{
  bool compare;

  assert (mpAttribute);
  
  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] > val);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<=(const Value& val)
{
  bool compare;

  assert (mpAttribute);
  
  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] <= val);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<(const Value& val)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] < val);

  mpAttribute = 0;

  return compare;
}

bool Record::operator+=(const Value& val)
{
  assert(mpAttribute);

  mpTableRecord->mpValues[mpAttribute->PositionGet()] += val;

  mpAttribute = 0;

  return TRUE; // TBD: Maybe return overflow instead
}

bool Record::operator-=(const Value& val)
{
  assert(mpAttribute);

  mpTableRecord->mpValues[mpAttribute->PositionGet()] -= val;

  mpAttribute = 0;

  return TRUE; // TBD: Maybe return underflow instead
}

bool Record::operator==(int i)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == i);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(int i)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] != i);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>=(int i)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] >= i);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>(int i)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] > i);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<=(int i)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] <= i);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<(int i)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] < i);

  mpAttribute = 0;

  return compare;
}

bool Record::operator+=(int i)
{
  assert(mpAttribute);

  mpTableRecord->mpValues[mpAttribute->PositionGet()] += i;

  mpAttribute = 0;

  return TRUE;
}

bool Record::operator-=(int i)
{
  assert(mpAttribute);

  mpTableRecord->mpValues[mpAttribute->PositionGet()] -= i;

  mpAttribute = 0;

  return TRUE;
}

bool Record::operator==(unsigned u)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == u);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(unsigned u)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] != u);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>=(unsigned u)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] >= u);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>(unsigned u)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] > u);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<=(unsigned u)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] <= u);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<(unsigned u)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] < u);

  mpAttribute = 0;

  return compare;
}

bool Record::operator+=(unsigned u)
{
  assert(mpAttribute);

  mpTableRecord->mpValues[mpAttribute->PositionGet()] += u;

  mpAttribute = 0;

  return TRUE; // TBD: maybe return overflow instead
}

bool Record::operator-=(unsigned u)
{
  assert(mpAttribute);

  mpTableRecord->mpValues[mpAttribute->PositionGet()] -= u;

  mpAttribute = 0;

  return TRUE; // TBD: maybe return underflow instead
}

bool Record::operator==(char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] != s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>=(char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] >= s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>(char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] > s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<=(char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] <= s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<(char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] < s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator==(const char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(const char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] != s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>=(const char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] >= s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>(const char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] > s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<=(const char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] <= s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<(const char* s)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] < s);

  mpAttribute = 0;

  return compare;
}

bool Record::operator==(const Bit& b)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == b);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(const Bit& b)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] != b);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>=(const Bit& b)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] >= b);

  mpAttribute = 0;

  return compare;
}

bool Record::operator>(const Bit& b)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] > b);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<=(const Bit& b)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] <= b);

  mpAttribute = 0;

  return compare;
}

bool Record::operator<(const Bit& b)
{
  bool compare;

  assert(mpAttribute);

  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] < b);

  mpAttribute = 0;

  return compare;
}

bool Record::operator==(void* ptr)
{
  bool compare;

  assert(mpAttribute);
  
  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == ptr);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(void* ptr)
{
  if (*this == ptr) return FALSE;
  else              return TRUE;
}

bool Record::operator==(bool b)
{
  bool compare;

  assert(mpAttribute);
  
  compare = (mpTableRecord->mpValues[mpAttribute->PositionGet()] == b);

  mpAttribute = 0;

  return compare;
}

bool Record::operator!=(bool b)
{
  if (*this == b) return FALSE;
  else            return TRUE;
}

bool Record::operator!()
{
  bool flag;

  flag = ! mpTableRecord->mpValues[mpAttribute->PositionGet()];

  mpAttribute = 0;

  return flag;
}

bool Record::operator=(const Value& val)
{
  bool ret = TRUE;

  assert (mpAttribute);

  // If the record belongs to a view then changing an attribute is done as a
  // transaction, otherwise, if it is a brand new record we can immediately
  // set the values.
  if (mAllowValUpdBeforeCommit
  ||  (mpView==0 || (mpView->mRecPermission|eRecPerm_OwnerAll)==eRecPerm_OwnerAll))
  {
    mpTableRecord->mpValues[mpAttribute->PositionGet()] = val;
    ret = TRUE;
  }
  else
    ret = mpView->TranRecordModify(this, mpAttribute, val);

  mpAttribute = 0;

  return ret;
}

bool Record::operator=(const char* str)
{
  Value v; M_Constructed(Value);

  v = str;

  return (*this) = v;
}

bool Record::operator=(void* vd)
{
  bool ret = TRUE;

  assert (mpAttribute);

  if (mAllowValUpdBeforeCommit || mpView==0) {
    mpTableRecord->mpValues[mpAttribute->PositionGet()] = vd;

    ret = TRUE;
  }
  else {
    Value val(vd); M_Constructed(Value);

    ret = mpView->TranRecordModify(this, mpAttribute, val);
  }

  mpAttribute = 0;

  return ret;
}

bool Record::operator=(unsigned u)
{
  bool ret = TRUE;

  assert (mpAttribute);

  if (mAllowValUpdBeforeCommit || mpView==0) {
    mpTableRecord->mpValues[mpAttribute->PositionGet()] = u;

    ret = TRUE;
  }
  else {
    Value val(u); M_Constructed(Value);

    ret = mpView->TranRecordModify(this, mpAttribute, val);
  }

  mpAttribute = 0;

  return ret;
}

bool Record::operator=(Bit& bit)
{
  bool ret = TRUE;

  assert (mpAttribute);

  if (mAllowValUpdBeforeCommit || mpView==0) {
    mpTableRecord->mpValues[mpAttribute->PositionGet()] = bit;

    ret = TRUE;
  }
  else {
    Value val(bit); M_Constructed(Value);

    ret = mpView->TranRecordModify(this, mpAttribute, val);
  }

  mpAttribute = 0;

  return ret;
}

void Record::Print(FILE* fp)
{
  short total = mpView->AttributesTotal();

  return Print(total, fp);
}

void Record::Print(short total, FILE* fp)
{
  for (short i=0; i<total; i++)
  {
    if (i==0) fprintf(fp, "< ");
    
    mpTableRecord->mpValues[i].Print(fp);

  //if      (i <  total-1) printf("%s", mpView->ValueSeparatorGet());
    if      (i <  total-1) fprintf(fp, "%s", ">|<");
    else if (i == total-1) fprintf(fp, " >");
  }

  fprintf(fp, "\n");
}

void Record::SetMsbLsb(short msb, short lsb)
{
  assert(mpAttribute->ValueType() == eValueBit);

  mpTableRecord->mpValues[mpAttribute->PositionGet()].SetMsbLsb(msb, lsb);

  // Do not clear mpAttribute as this is an intermediate
  // function before mpAttribute is consumed.
}

void Record::ValueCopyGet(const Attribute* attr, Value &value)
{
  value = mpTableRecord->mpValues[attr->PositionGet()];
}

void Record::ValueCopyGet(short pos, Value &value)
{
  value = mpTableRecord->mpValues[pos];
}

void Record::ValueCopyGet(const Attribute* attr, Bit &bit)
{
  mpTableRecord->mpValues[attr->PositionGet()].ValueCopyGet(bit);
}

void Record::ValueCopyGet(short pos, Bit &bit)
{
  mpTableRecord->mpValues[pos].ValueCopyGet(bit);
}

void* Record::GetValueAsPointer(const Attribute* attr)
{
  assert (attr->ValueType() == eValueVoidPtr);

  return (void*)(mpTableRecord->mpValues[attr->PositionGet()]);
}

unsigned Record::GetValueAsUnsigned(const Attribute* attr)
{
  assert (attr->ValueType() == eValueUnsigned);

  return (unsigned)(mpTableRecord->mpValues[attr->PositionGet()]);
}

// Gived problems in Interface class.
//Record::operator unsigned()
//{
//  assert(mpAttribute);
//
//  return (unsigned)(mpTableRecord->mpValues[mpAttribute->PositionGet()]);
//}

bool Record::ViewSet(View* v)
{
  if (mpView == 0) { mpView = v; return TRUE;  }
  else             {             return FALSE; } // view already set
}

void Record::p() { Print(); }

// End
