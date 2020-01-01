// $RCSfile: Condition.cc,v $
// $Revision: 1.9 $

#include <assert.h>

#include "Condition.h"
//
#include "Aggregate.h"
#include "Attribute.h"
#include "BucketList.h"
#include "Database.h"
#include "Enum.h"
#include "Global.h"
#include "Record.h"
#include "Value.h"
#include "View.h"

M_ExternConstructDestructCountInt;

NameVal OperatorNV[] =
{
  eCondOper_None,            "none",
  eCondOper_Less,            "<",
  eCondOper_Greater,         ">",
  eCondOper_Equal,           "==",
  eCondOper_NotEqual,        "!=",
  eCondOper_GreaterOrEqual,  ">=",
  eCondOper_LessOrEqual,     "<=",
  eCondOper_Like,            "like",
  eCondOper_Negation,        "negation",
  eCondOper_Invalid,         "invalid"
};

static bool gDbg = 0;

Condition::Condition(const Attribute* attr) :
  mTypeLHS             (eCondOperand_AttributePtr),
  mTypeRHS             (eCondOperand_Unknown),
  mpAttributeLHS       (attr),
  mpAttributeRHS       (0),
  mpValueLHS           (0),
  mpValueRHS           (0),
  mpViewIteratorLHS    (0),
  mpViewIteratorRHS    (0),
  mCondOperator        (eCondOper_None),
  mLsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mMsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mBitWidthSet         (FALSE),
  mIsAllocatedAuto     (FALSE),
  mValueIndex          (0)
{
  assert(mpAttributeLHS);

  M_UpdConstructCount(eType_Condition);
}

Condition::Condition(const Aggregate* aggr) :
  mTypeLHS             (eCondOperand_Unknown),
  mTypeRHS             (eCondOperand_Unknown),
  mpAttributeLHS       (0),
  mpAttributeRHS       (0),
  mpValueLHS           (0),
  mpValueRHS           (0),
  mpViewIteratorLHS    (0),
  mpViewIteratorRHS    (0),
  mCondOperator        (eCondOper_None),
  mLsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mMsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mBitWidthSet         (FALSE),
  mIsAllocatedAuto     (FALSE),
  mValueIndex          (0)
{
  assert(aggr);

  mpValueLHS = ValuePtrGet(aggr);

  mTypeLHS   = eCondOperand_ValuePtr;

  M_UpdConstructCount(eType_Condition);
}

Condition::Condition(const Value val) :
  mTypeLHS             (eCondOperand_Value),
  mTypeRHS             (eCondOperand_Unknown),
  mpAttributeLHS       (0),
  mpAttributeRHS       (0),
  mValueLHS            (val),
  mpValueLHS           (0),
  mpValueRHS           (0),
  mpViewIteratorLHS    (0),
  mpViewIteratorRHS    (0),
  mCondOperator        (eCondOper_None),
  mLsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mMsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mBitWidthSet         (FALSE),
  mIsAllocatedAuto     (FALSE),
  mValueIndex          (0)
{
  M_UpdConstructCount(eType_Condition);
}

Condition::Condition(const Value* pval) :
  mTypeLHS             (eCondOperand_ValuePtr),
  mTypeRHS             (eCondOperand_Unknown),
  mpAttributeLHS       (0),
  mpAttributeRHS       (0),
  mpValueLHS           (pval),
  mpValueRHS           (0),
  mpViewIteratorLHS    (0),
  mpViewIteratorRHS    (0),
  mCondOperator        (eCondOper_None),
  mLsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mMsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mBitWidthSet         (FALSE),
  mIsAllocatedAuto     (FALSE),
  mValueIndex          (0)
{
  assert(mpValueLHS);

  M_UpdConstructCount(eType_Condition);
}

Condition::Condition(const Attribute* attr, bool alloc) :
  mTypeLHS             (eCondOperand_AttributePtr),
  mTypeRHS             (eCondOperand_Unknown),
  mpAttributeLHS       (attr),
  mpAttributeRHS       (0),
  mpValueLHS           (0),
  mpValueRHS           (0),
  mpViewIteratorLHS    (0),
  mpViewIteratorRHS    (0),
  mCondOperator        (eCondOper_None),
  mLsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mMsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mBitWidthSet         (FALSE),
  mIsAllocatedAuto     (alloc),
  mValueIndex          (0)
{
  assert(mpAttributeLHS);

  M_UpdConstructCount(eType_Condition);
}

Condition::Condition(const Aggregate* aggr, bool alloc) :
  mTypeLHS             (eCondOperand_Unknown),
  mTypeRHS             (eCondOperand_Unknown),
  mpAttributeLHS       (0),
  mpAttributeRHS       (0),
  mpValueLHS           (0),
  mpValueRHS           (0),
  mpViewIteratorLHS    (0),
  mpViewIteratorRHS    (0),
  mCondOperator        (eCondOper_None),
  mLsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mMsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mBitWidthSet         (FALSE),
  mIsAllocatedAuto     (alloc),
  mValueIndex          (0)
{
  assert(aggr);

  mpValueLHS = ValuePtrGet(aggr);
  mTypeLHS   = eCondOperand_ValuePtr;

  M_UpdConstructCount(eType_Condition);
}

Condition::Condition(const Value& val, bool alloc) :
  mTypeLHS             (eCondOperand_Value),
  mTypeRHS             (eCondOperand_Unknown),
  mpAttributeLHS       (0),
  mpAttributeRHS       (0),
  mValueLHS            (val),
  mpValueLHS           (0),
  mpValueRHS           (0),
  mpViewIteratorLHS    (0),
  mpViewIteratorRHS    (0),
  mCondOperator        (eCondOper_None),
  mLsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mMsbBit              (SHRT_MAX), // set for max surprise, faster debug
  mBitWidthSet         (FALSE),
  mIsAllocatedAuto     (alloc),
  mValueIndex          (0)
{
  assert(mpAttributeLHS);

  M_UpdConstructCount(eType_Condition);
}

Condition::Condition(const Condition& c) :
  mTypeLHS             (c.mTypeLHS),
  mTypeRHS             (c.mTypeRHS),
  mpAttributeLHS       (c.mpAttributeLHS),
  mpAttributeRHS       (c.mpAttributeRHS),
  mpValueLHS           (c.mpValueLHS),
  mpValueRHS           (c.mpValueRHS),
  mpViewIteratorLHS    (c.mpViewIteratorLHS),
  mpViewIteratorRHS    (c.mpViewIteratorRHS),
  mCondOperator        (c.mCondOperator),
  mLsbBit              (c.mLsbBit),
  mMsbBit              (c.mMsbBit),
  mBitWidthSet         (c.mBitWidthSet),
  mIsAllocatedAuto     (FALSE),
  mValueIndex          (c.mValueIndex)
{
  mValueLHS = c.mValueLHS;
  mValueRHS = c.mValueRHS;

  M_UpdConstructCount(eType_Condition);
}

Condition::~Condition()
{
  M_UpdDestructCount(eType_Condition);
}

bool Condition::Match(Record* rec)
{
  // This Match handles non-join queries. All conditions are expected to contain
  // just one Attribute which will be for the LHS. The RHS will be a value or a
  // value pointer.

  if (! rec) {
    return Evaluate();
  }
  else {
    if (gDbg) {
      char buf1[MAX_RECORD_LINE_LENGTH+1];
      char buf2[MAX_RECORD_LINE_LENGTH+1];

      Value val;
      rec->ValueCopyGet(&AttributeGet(), val);
      val.Save(buf1);
      ValueGet().Save(buf2);

    //Print();
    //printf("Checking %s %s %s\n", buf1, Enum2Str(OperatorNV, Operator()), buf2);
    }

    if (AttributeGet().ValueType()==eValueBit && BitWidthSet()) {
      rec->SetMsbLsb(GetMsb(), GetLsb());
    }

    return Evaluate(rec);
  }
}

// This Evaluate() requires no args which means it already has
// the lhs and rhs operands available (else we assert an error).
bool Condition::Evaluate()
{
  const Value* lhs;
  const Value* rhs;

  switch (mTypeLHS) {
  case eCondOperand_Value:        lhs = &mValueLHS; break;
  case eCondOperand_ValuePtr:     lhs = mpValueLHS; break;
  case eCondOperand_AttributePtr: assert(mpViewIteratorLHS);
                                  lhs =(mpViewIteratorLHS->RecordCurrentGet())->ValueGet(mpAttributeLHS);
                                  assert(lhs);
                                  break;
  default:                        assert(0);
  }

  switch (mTypeRHS) {
  case eCondOperand_Value:        rhs = &mValueRHS;
  case eCondOperand_ValuePtr:     rhs = mpValueRHS;
  case eCondOperand_AttributePtr: assert(mpViewIteratorRHS);
                                  rhs =(mpViewIteratorRHS->RecordCurrentGet())->ValueGet(mpAttributeRHS);
                                  assert(rhs); break;
  default:                        assert(0);
  }

  return Evaluate(*lhs, *rhs);
}

bool Condition::Evaluate(Record* rec)
{
  MacroAssert1(rec);

  // Only one attribute (lhs or rhs) should be set.
  MacroAssert1(!mpAttributeLHS || !mpAttributeRHS);

  const Value* lhs;
  const Value* rhs;

  switch (mTypeLHS) {
  case eCondOperand_Value:        lhs = &mValueLHS; break;
  case eCondOperand_ValuePtr:     lhs = mpValueLHS; break;
  case eCondOperand_AttributePtr: lhs = rec->ValueGet(mpAttributeLHS); break;
  default:                        assert(0);
  }

  switch (mTypeRHS) {
  case eCondOperand_Value:        rhs = &mValueRHS; break;
  case eCondOperand_ValuePtr:     rhs = mpValueRHS; break;
  case eCondOperand_AttributePtr: rhs = rec->ValueGet(mpAttributeRHS); break;
  //
  case eCondOperand_RecordPtr:    if (mCondOperator==eCondOper_Equal)
                                    return (Record*)mpValueRHS == rec;
                                  if (mCondOperator==eCondOper_NotEqual)
                                    return (Record*)mpValueRHS != rec;
                                  assert(0); break;
  //
  default:                        assert(0);
  }

  return Evaluate(*lhs, *rhs);
}

bool Condition::Evaluate(const Value& lhs, const Value& rhs)
{
  switch (mCondOperator)
  {
  case eCondOper_None:             return TRUE;
  case eCondOper_Less:             return (lhs <  rhs);
  case eCondOper_Greater:          return (lhs >  rhs);
  case eCondOper_Equal:            return (lhs == rhs);
  case eCondOper_NotEqual:         return (lhs != rhs);
  case eCondOper_GreaterOrEqual:   return (lhs >= rhs);
  case eCondOper_LessOrEqual:      return (lhs <= rhs);
  case eCondOper_Like:             assert (0); // until supported
  case eCondOper_Negation:         return (    !  lhs);
  default:                         assert (0);
  }
}

Condition& Condition::operator&&(Condition& c)
{
  BucketList<Condition> &bl1 =   mConditionBktList;
  BucketList<Condition> &bl2 = c.mConditionBktList;

  OperationAnd(bl1, bl2);

  // Now that all conditions in bl2 are added to bl1 using DeepCopy,
  // we should free any automatically allocated conditions in bl2.
  if (c.mIsAllocatedAuto) delete &c;

  return *this;
}

Condition& Condition::operator||(Condition& c)
{
  BucketList<Condition> &bl1 =   mConditionBktList;
  BucketList<Condition> &bl2 = c.mConditionBktList;

  OperationOr(bl1, bl2);

  // Now that all conditions in bl2 are added to bl1 using DeepCopy,
  // we should free any automatically allocated conditions in bl2.
  if (c.mIsAllocatedAuto) delete &c;

  return *this;
}

void OperationOr(BucketList<Condition>& bl1, BucketList<Condition>& bl2)
{
  BucketList<Condition> qbl;

  qbl.CopyDeep(bl2);
  bl1.AttachDestroy(qbl, eSouth);
}

// Initializing Condition with Value's.
Condition& Condition::operator<(const Value& val)
{
  mCondOperator  = eCondOper_Less;
  mValueRHS      = val;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>(const Value& val)
{
  mCondOperator  = eCondOper_Greater;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>=(const Value& val)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<=(const Value& val)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator==(const Value& val)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(const Value& val)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator%=(const Value& val)
{
  mCondOperator  = eCondOper_Like;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

// Initializing Condition with Value pointer's.
Condition& Condition::operator<(const Value* pval)
{
  mCondOperator  = eCondOper_Less;
  mpValueRHS     = pval;
  mTypeRHS       = eCondOperand_ValuePtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>(const Value* pval)
{
  mCondOperator  = eCondOper_Greater;
  mpValueRHS     = pval;
  mTypeRHS       = eCondOperand_ValuePtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>=(const Value* pval)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mpValueRHS     = pval;
  mTypeRHS       = eCondOperand_ValuePtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<=(const Value* pval)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mpValueRHS     = pval;
  mTypeRHS       = eCondOperand_ValuePtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator==(const Value* pval)
{
  mCondOperator  = eCondOper_Equal;
  mpValueRHS     = pval;
  mTypeRHS       = eCondOperand_ValuePtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(const Value* pval)
{
  mCondOperator  = eCondOper_NotEqual;
  mpValueRHS     = pval;
  mTypeRHS       = eCondOperand_ValuePtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator%=(const Value* pval)
{
  mCondOperator  = eCondOper_Like;
  mpValueRHS     = pval;
  mTypeRHS       = eCondOperand_ValuePtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

// Initializing Condition with int's.
Condition& Condition::operator<(int val)
{
  mCondOperator  = eCondOper_Less;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>(int val)
{
  mCondOperator  = eCondOper_Greater;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>=(int val)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<=(int val)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator==(int val)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(int val)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

// Initializing Condition with unsigned int's.
Condition& Condition::operator<(unsigned int val)
{
  mCondOperator  = eCondOper_Less;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>(unsigned int val)
{
  mCondOperator  = eCondOper_Greater;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>=(unsigned int val)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<=(unsigned int val)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator==(unsigned int val)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(unsigned int val)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

// Initializing Condition with short's.
Condition& Condition::operator<(short val)
{
  mCondOperator  = eCondOper_Less;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>(short val)
{
  mCondOperator  = eCondOper_Greater;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>=(short val)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<=(short val)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator==(short val)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(short val)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

// Initializing Condition with unsigned short.
Condition& Condition::operator<(unsigned short val)
{
  mCondOperator  = eCondOper_Less;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>(unsigned short val)
{
  mCondOperator  = eCondOper_Greater;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>=(unsigned short val)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<=(unsigned short val)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator==(unsigned short val)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(unsigned short val)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<(const Bit& b)
{
  mCondOperator  = eCondOper_Less;
  mValueRHS      = b;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

Condition& Condition::operator>(const Bit& b)
{
  mCondOperator  = eCondOper_Greater;
  mValueRHS      = b;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

Condition& Condition::operator>=(const Bit& b)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mValueRHS      = b;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

Condition& Condition::operator<=(const Bit& b)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mValueRHS      = b;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

Condition& Condition::operator==(const Bit& b)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = b;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

Condition& Condition::operator!=(const Bit& b)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = b;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

// Initializing Condition with const char* .
Condition& Condition::operator<(const char* val)
{
  mCondOperator  = eCondOper_Less;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>(const char* val)
{
  mCondOperator  = eCondOper_Greater;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator>=(const char* val)
{
  mCondOperator  = eCondOper_GreaterOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator<=(const char* val)
{
  mCondOperator  = eCondOper_LessOrEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator==(const char* val)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(const char* val)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(const Record* rec)
{
  mCondOperator  = eCondOper_NotEqual;
  mpValueRHS     = (Value*)rec;
  mTypeRHS       = eCondOperand_RecordPtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

Condition& Condition::operator==(const Record* rec)
{
  mCondOperator  = eCondOper_Equal;
  mpValueRHS     = (Value*)rec;
  mTypeRHS       = eCondOperand_RecordPtr;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

// Initializing Condition with Attributes
Condition& Condition::operator<(const Attribute& attr)
{
  mCondOperator      = eCondOper_Less;
  mpAttributeRHS     = &attr;
  mTypeRHS           = eCondOperand_AttributePtr;

  return *this;
}

Condition& Condition::operator>(const Attribute& attr)
{
  mCondOperator      = eCondOper_Greater;
  mpAttributeRHS     = &attr;
  mTypeRHS           = eCondOperand_AttributePtr;

  return *this;
}

Condition& Condition::operator>=(const Attribute& attr)
{
  mCondOperator      = eCondOper_GreaterOrEqual;
  mpAttributeRHS     = &attr;
  mTypeRHS           = eCondOperand_AttributePtr;

  return *this;
}

Condition& Condition::operator<=(const Attribute& attr)
{
  mCondOperator      = eCondOper_LessOrEqual;
  mpAttributeRHS     = &attr;
  mTypeRHS           = eCondOperand_AttributePtr;

  return *this;
}

Condition& Condition::operator==(const Attribute& attr)
{
  mCondOperator      = eCondOper_Equal;
  mpAttributeRHS     = &attr;
  mTypeRHS           = eCondOperand_AttributePtr;

  return *this;
}

Condition& Condition::operator!=(const Attribute& attr)
{
  mCondOperator      = eCondOper_NotEqual;
  mpAttributeRHS     = &attr;
  mTypeRHS           = eCondOperand_AttributePtr;

  return *this;
}

// Initializing with void's.
Condition& Condition::operator==(void* val)
{
  mCondOperator  = eCondOper_Equal;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!=(void* val)
{
  mCondOperator  = eCondOper_NotEqual;
  mValueRHS      = val;
  mTypeRHS       = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);
  return *this;
}

Condition& Condition::operator!()
{
  mCondOperator  = eCondOper_Negation;

  Value val(eValueTypeDontCare); M_Constructed(Value);

  mValueRHS = val;
  mTypeRHS  = eCondOperand_Value;

  mConditionBktList.Clean();
  mConditionBktList.Add(this, eSouth);

  return *this;
}

void Condition::Print(FILE* fp)
{
  fprintf(fp, "%s %s ", mpAttributeLHS->Name(), Enum2Str(OperatorNV, mCondOperator));
  mValueRHS.Print(fp);
}

void Condition::Dump(FILE* fp)
{
  Condition* c;
  short count = 0;

  BucketList<Condition>& ql = mConditionBktList;

  for (ql.BktSetPosition(ePositionFirst); !ql.BktDone(); )
  {
    fprintf(fp, "Bucket %d\n", ++count);

    for (ql.SetLinkPosition(ePositionFirst); !ql.LinkDone(); )
    {
      if (c=ql.Get()) c->Print(fp);

      ql.SetLinkPosition(ePositionNext);
    }

    ql.BktSetPosition(ePositionNext);
  }
}

Condition& Condition::operator[](const char* width_str)
{
  short lsb, msb, temp;

  if (width_str != 0) {

    int ret = sscanf(width_str, "%hd:%hd", &msb, &lsb);

    if (ret == 1) lsb = msb;

    assert(ret);

    if (ret==1 || ret==2) {
      mLsbBit = lsb;
      mMsbBit = msb;

      mBitWidthSet = TRUE;
    }

    if (mLsbBit > mMsbBit) {
      temp    = mLsbBit;
      mLsbBit = mMsbBit;
      mMsbBit = temp;
    }
  }

  return *this;
}

void OperationAnd(BucketList<Condition>& bl1, BucketList<Condition>& bl2)
{
  // The && operation for conditions is essentially the multiply operation.
  // Example: Cx=c0||c1, Cy=c2||c3&&c4, (Cx=2rows,2elems, Cy=2rows,3elems)
  //          if we use . for && and use + for || we can rewrite this as:
  //          Cx=c0+c1,  Cy=c2+c3.c4
  // then     Cx&&Cy (ie Cx.Cy)
  // equals = (c0+c1).(c2+c3.c4)
  //        = (c0+c1).c2 + (c0+c1).c3.c4
  //        = c0.c2 + c1.c2 + c0.c3.c4 + c1.c3.c4
  // by simply expanding the terms, the resulting bucketlist has 4rows,10elems.
  //
  // We implement this by REPLICATING the rows of Cx N-1 times where N is the
  // number of rows in Cy and APPEND each row of Cy to Cx.
  // Note that if there is only one row in Cy then only append is needed.

  bool need_to_replicate;

  if (bl2.TotalBuckets() > 1) need_to_replicate = TRUE;
  else                        need_to_replicate = FALSE;

  bl1.BktSetPosition(ePositionFirst);

  for (; !bl1.BktDone(); bl1.BktSetPosition(ePositionNext)) {
    Bucket<Condition> *bkt_first;

    bl2.BktSetPosition(ePositionFirst);

    for (; !bl2.BktDone(); bl2.BktSetPosition(ePositionNext)) {
      if (bl2.BktGetPosition() == ePositionFirst) {
        // No replication.
        // Check if we will need a copy of the first bucket for replication.
        if (need_to_replicate) {
          bkt_first  = new Bucket<Condition>;
          // Note: Using deep copy allows a simpler 'delete' scheme, we simply
          //       delete all objects in the BucketList.
          //       This is not possible with shallow copy. However, shallow copy
          //       would have been more efficient (using less memory).
          //       Remember that the original Condition objects are
          //       automatically added by the Attribute compare operators and so
          //       we were anyway responsible for freeing the Condition objects
          //       all along.
          bkt_first->CopyDeep(*bl1.BktGet(ePositionCurrent));
        }
      }
      else {
        // Replicate.
        // For the last iteration use up the 'bkt_first' copy we already have.
        if (bl2.BktGetPosition() == ePositionLast && need_to_replicate) {
          bl1.BktAdd(bkt_first, ePositionNext); // insert bucket
        }
        else {
          Bucket<Condition> *bkt = new Bucket<Condition>;
          // See note on deep copy above.
          bkt->CopyDeep(*bkt_first);
          bl1.BktAdd(bkt, ePositionNext); // insert bucket
        }
      }

      // Append current bl bucket to this current bucket.
      bl1.BktAppend(bl2);
    }
  }
}

Condition& Condition::EditIndex(short s)
{
  mValueIndex = s;

  return *this;
}

// Friend function in Aggregate needed to access private: mpValue
Value* Condition::ValuePtrGet(const Aggregate* aggr) const
{
  return aggr->mpValue;
}

// End
