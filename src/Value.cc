// $RCSfile: Value.cc,v $
// $Revision: 1.6 $

#include <assert.h>
#include <limits.h>
#include <string.h>

#include "Value.h"
//
#include "Bit.h"
#include "BucketList.h"
#include "Condition.h"
#include "Global.h"

M_ExternConstructDestructCountInt;

NameVal ValueTypeNV[] =
{
  eValueUnknown,            "Type:???",
  eValueBit,                "Type:Bit",
  eValueChar,               "Type:Char",
  eValueCharPtr,            "Type:CharPtr",
  eValueConstCharConstPtr,  "Type:ConstCharPtr",
  eValueString,             "Type:String",
  eValueUnsigned,           "Type:Unsigned",
  eEnumInvalid,             ""
};

Value::Value() :
  mNewCalled           (FALSE),
  mType                (eValueUnknown),
  mpValConstCharConst  (0)
{
  // This constructor is mainly needed for Value arrays.

  // No values initialized, they remain unknown
  // (except that compiler will initialize to some default value).

  M_UpdConstructCount(eType_Value);
}

Value::Value(eValueType type) :
  mType                  (type),
  mNewCalled             (FALSE),
  mpValConstCharConst    (0)
{
  // type is known but value is not, initialize to a reasonable value.
  switch (type) {
    case eValueUnknown:                                       break;
    case eValueBit:               mpValBit            = 0;    break;
    case eValueChar:              mValChar            = '\0'; break;
    case eValueCharPtr:           mpValChar           = 0;    break;
    case eValueConstCharConstPtr:                             break;
    case eValueVoidPtr:           mpValVoid           = 0;    break;
    case eValueString:            mpValString         = 0;    break;
    case eValueUnsigned:          mValUnsigned        = 0;    break;
    case eValueInt:               mValInt             = 0;    break;
    default:                      assert(0);
  }

  M_UpdConstructCount(eType_Value);
}

Value::Value(short v) :
  mType       (eValueInt),
  mNewCalled  (FALSE)
{
  mValInt = v;

  M_UpdConstructCount(eType_Value);
}

Value::Value(unsigned short v) :
  mType        (eValueUnsigned),
  mValUnsigned (v),
  mNewCalled   (FALSE)
{
  M_UpdConstructCount(eType_Value);
}

Value::Value(int v) :
  mType        (eValueInt),
  mValInt      (v),
  mNewCalled   (FALSE)
{
  M_UpdConstructCount(eType_Value);
}

Value::Value(unsigned int v) :
  mType         (eValueUnsigned),
  mValUnsigned  (v),
  mNewCalled    (FALSE)
{
  M_UpdConstructCount(eType_Value);
}

Value::Value(char v) :
  mType       (eValueChar),
  mValChar    (v),
  mNewCalled  (FALSE)
{
  M_UpdConstructCount(eType_Value);
}

Value::Value(const char *v) :
  mType       (eValueCharPtr),
  mpValChar   (0),
  mNewCalled  (FALSE)
{
  mpValChar = new char [strlen(v)+1]; M_UpdConstructCountN(eType_Char, strlen(v)+1);

  if (mpValChar) {
    mNewCalled = TRUE;

    strcpy(mpValChar, v);
  }

  M_UpdConstructCount(eType_Value);
}

Value::Value(const char* & v) :
  mType                 (eValueConstCharConstPtr),
  mpValConstCharConst   (v),
  mNewCalled            (FALSE)
{
  M_UpdConstructCount(eType_Value);
}

Value::Value(const String* v) :
  mType         (eValueString),
  mpValString   (0),
  mNewCalled    (FALSE)
{
  if (v != 0) {
    mpValString = new String(*v);

    if (mpValString) mNewCalled = TRUE;
  }

  M_UpdConstructCount(eType_Value);
}

Value::Value(const String& v) :
  mType         (eValueString),
  mpValString   (0),
  mNewCalled    (FALSE)
{
  mpValString = new String(v);

  if (mpValString) {
    mNewCalled = TRUE;
  }

  M_UpdConstructCount(eType_Value);
}

// CONT: May need to store msb, lsb of Bit in Value so they dont get cleared
//       after a Value comparison (needed iff msb, lsb is set differently).
Value::Value(const Bit* b) :
  mType       (eValueBit),
  mpValBit    (0),
  mNewCalled  (FALSE)
{
  if (b) {
    mpValBit = new Bit(*b);

    if (mpValBit) mNewCalled = TRUE;
  }

  M_UpdConstructCount(eType_Value);
}

Value::Value(const Bit& b) :
  mType       (eValueBit),
  mpValBit    (0),
  mNewCalled  (FALSE)
{
  // TBD: May be always better to assign mType after allocation is successful.

  mpValBit = new Bit(b);
  M_Constructed(Bit);

  if (mpValBit != 0) mNewCalled = TRUE;

  assert (mpValBit);

  M_UpdConstructCount(eType_Value);
}

Value::Value(void* v) :
  mType       (eValueVoidPtr),
  mpValVoid   (v),
  mNewCalled  (FALSE)
{
  M_UpdConstructCount(eType_Value);
}

Value::Value(const Value& v) :
  mType       (v.mType),
  mNewCalled  (FALSE)
{
  *this = v;

  M_UpdConstructCount(eType_Value);
}

Value::~Value()
{
  ValueFree();

  M_UpdDestructCount(eType_Value);
}

unsigned Value::Hash()
{
  unsigned hash = 0;

  return hash;
}

Value& Value::operator++()
{
  if      (mType == eValueUnsigned)  mValUnsigned++;
  else if (mType == eValueInt)       mValInt++;
  else if (mType == eValueBit)      *mpValBit++;
  else                               assert(0);

  return *this;
}

Value& Value::operator++(int)
{
  if      (mType == eValueUnsigned)  mValUnsigned++;
  else if (mType == eValueInt)       mValInt++;
  else if (mType == eValueBit)      *mpValBit++;
  else                               assert(0);

  return *this;
}

Value& Value::operator--()
{
  if      (mType == eValueUnsigned)  mValUnsigned--;
  else if (mType == eValueInt)       mValInt--;
  else if (mType == eValueBit)      *mpValBit--;
  else                               assert(0);

  return *this;
}

Value& Value::operator--(int)
{
  if      (mType == eValueUnsigned)  mValUnsigned--;
  else if (mType == eValueInt)       mValInt--;
  else if (mType == eValueBit)      *mpValBit--;
  else                               assert(0);

  return *this;
}

// For strings this does concatenation.
Value& Value::operator+(const Value& val)
{
  if (mType==eValueString && val.mType==eValueString)
  {
    *mpValString << *val.mpValString;
    return *this;
  }

  assert(   (mType==eValueUnsigned && val.mType==eValueUnsigned)
         || (mType==eValueInt      && val.mType==eValueInt)
         || (mType==eValueBit      && val.mType==eValueBit));

  if (mType == eValueBit) {
    *mpValBit = *mpValBit + *(val.mpValBit);
    return *this;
  }

  // TBD: add overflow check.
  if      (mType == eValueUnsigned) mValUnsigned += val.mValUnsigned;
  else if (mType == eValueInt)      mValInt      += val.mValInt;
  else                              assert(0);

  return *this;
}

Value& Value::operator+=(const Value& val)
{
  return (*this + val);
}

Value& Value::operator-(const Value& val)
{
  assert(mType==eValueUnsigned && val.mType==eValueUnsigned);

  assert (   (mType==eValueUnsigned && val.mType==eValueUnsigned)
          || (mType==eValueInt      && val.mType==eValueInt)
          || (mType==eValueBit      && val.mType==eValueBit));

  if (mType == eValueBit) {
    *mpValBit = *mpValBit - *(val.mpValBit);
    return *this;
  }

  // TBD: add underflow check.
  if      (mType == eValueUnsigned) mValUnsigned -= val.mValUnsigned;
  else if (mType == eValueInt)      mValInt      -= val.mValInt;
  else                              assert(0);

  return *this;
}

Value& Value::operator-=(const Value& val)
{
  return (*this - val);
}

Value& Value::operator*(const Value& val)
{
  assert(   (mType==eValueUnsigned && val.mType==eValueUnsigned)
         || (mType==eValueInt      && val.mType==eValueInt));
  
  if      (mType==eValueUnsigned) mValUnsigned = mValUnsigned*(val.mValUnsigned);
  else if (mType==eValueUnsigned) mValInt      = mValInt     *(val.mValInt);
  else                            assert(0);

  return *this;
}

Value& Value::operator/(const Value& val)
{
  assert(   (mType==eValueUnsigned && val.mType==eValueUnsigned)
         || (mType==eValueInt      && val.mType==eValueInt));

  if      (mType==eValueUnsigned) mValUnsigned = mValUnsigned/(val.mValUnsigned);
  else if (mType==eValueInt)      mValInt      = mValInt/(val.mValInt);

  return *this;
}

Value& Value::operator+(unsigned u)
{
  assert (mType==eValueUnsigned || mType==eValueBit || mType==eValueInt);

  if (mType == eValueBit) {
    *mpValBit = *mpValBit + u;
    return *this;
  }

  // TBD: check for overflow.
  if      (mType==eValueUnsigned) mValUnsigned += u;
  else if (mType==eValueInt)      mValInt      += u;

  return *this;
}

Value& Value::operator+=(unsigned u)
{
  return (*this + u);
}

Value& Value::operator-(unsigned u)
{
  assert(mType==eValueUnsigned || mType==eValueBit || mType==eValueInt);

  if (mType == eValueBit) {
    *mpValBit = *mpValBit - u;
    return *this;
  }

  // TBD: check for underflow.
  if      (mType==eValueUnsigned) mValUnsigned -= u;
  else if (mType==eValueInt)      mValInt      -= u;

  return *this;
}

Value& Value::operator-=(unsigned u)
{
  return (*this - u);
}

Value& Value::operator*(unsigned u)
{
  assert(mType==eValueUnsigned || mType==eValueInt);
  
  // TBD: check for overflow.
  if      (mType==eValueUnsigned) mValUnsigned = mValUnsigned*u;
  else if (mType==eValueInt)      mValInt      = mValInt*u;
  else    assert(0);

  return *this;
}

Value& Value::operator/(unsigned u)
{
  assert(mType==eValueUnsigned || mType==eValueInt);

  if      (mType==eValueUnsigned) mValUnsigned = mValUnsigned/u;
  else if (mType==eValueInt)      mValInt      = mValInt/u;
  else    assert(0);

  return *this;
}

Value& Value::operator=(const Value& v)
{
  // Before overwriting, free existing value if it was allocated,
  // and check if we can optimize.
  // We can optimize if the new object is of the same mType (and was
  // also allocated) in which case we leave it to the class assignment
  // operator to determine how to handle the assignment as there are
  // cases for optimization where we can save an explicit 'free' and
  // 'new' sequence. For example when assigning Bits of equal bit size.

  bool optimize_possible = FALSE;

  if (mNewCalled) {
    if (v.mNewCalled) {
      if ((mType==eValueString && v.mType == eValueString)
      ||  (mType==eValueBit    && v.mType == eValueBit))
      {
        optimize_possible = TRUE;
      }
    }

    if (!optimize_possible) ValueFree();
  }
  // else: no need to free, nothing allocated.

  mType      = v.mType;
//mNewCalled = v.mNewCalled; // not sure this is good 9/18/09

  switch (v.mType) {
    case eValueUnsigned:
      mValUnsigned = v.mValUnsigned;
      break;
    case eValueInt:
      mValInt = v.mValInt;
      break;
    case eValueCharPtr:
      if (v.mpValChar == 0) {
        mpValChar = 0;
      }
      else {
        mpValChar = new char [strlen(v.mpValChar)+1]; M_UpdConstructCountN(eType_Char, strlen(v.mpValChar)+1);
        if (mpValChar) {
          mNewCalled = TRUE;

          strcpy(mpValChar, v.mpValChar);
        }
      }
      break;
    case eValueConstCharConstPtr:
      mpValConstCharConst = v.mpValConstCharConst;
      break;
    case eValueChar:
      mValChar = v.mValChar;
      break;
    case eValueString:
      if (optimize_possible) {
        *mpValString = *v.mpValString;
      }
      else {
        // If v allocated the string then follow same policy here.
        if (v.mNewCalled) {
          mpValString = new String(*v.mpValString);

          mNewCalled = TRUE;
        }
        else
          mpValString = v.mpValString;
      }
      break;
    case eValueBit:
      if (optimize_possible) {
        *mpValBit = *v.mpValBit;
      }
      else {
        // If v allocated the bit then follow same policy here.
        if (v.mNewCalled) {
          mpValBit = new Bit(*v.mpValBit);
          M_Constructed(Bit);

          mNewCalled = TRUE;
        }
        else
          mpValBit = v.mpValBit;
      }
      break;
    case eValueVoidPtr:
      mpValVoid = v.mpValVoid;
      break;
    default:
      assert(0);
      break;
  }

  return *this;
}

Value& Value::operator=(short v)
{
  Value val(v);

  *this = val; // operator= will take care of delete if needed

  return *this;
}

Value& Value::operator=(unsigned short v)
{
  Value val(v);

  *this = val; // operator= will take care of delete if needed

  return *this;
}

Value& Value::operator=(int v)
{
  if (mType == eValueInt
  ||  mType == eValueUnknown)
  {
    mType = eValueInt;

    mValInt = v;

    return *this;
  }

  Value val(v); M_Constructed(Value);

  *this = val; // operator= will take care of delete if needed

  return *this;
}

Value& Value::operator=(unsigned int v)
{
  // FIX1:
  if (mType == eValueUnsigned
  ||  mType == eValueUnknown)
  {
    mType = eValueUnsigned;

    mValUnsigned = v;

    return *this;
  }

  Value val(v); M_Constructed(Value);

  *this = val; // operator= will take care of delete if needed

  return *this;
}

Value& Value::operator=(char v)
{
  Value val(v); M_Constructed(Value);

  *this = val; // operator= will take care of delete if needed

  return *this;
}

Value& Value::operator=(const char *v)
{
  Value val((const char*)v); M_Constructed(Value);

  *this = val; // operator= will take care of delete if needed

  return *this;
}

Value& Value::operator=(Bit& v)
{
  if (mType==eValueBit
  ||  mType==eValueUnknown)
  {
    if (mType == eValueUnknown
    || (mType == eValueBit && mpValBit==0))
    {
      mpValBit = new Bit(v.BitWidth());
      M_Constructed(Bit);

      if (mpValBit) mNewCalled = TRUE;
    }

    mType = eValueBit;

    assert(mpValBit);

    *mpValBit = v;

    return *this;
  }

  Value val(v); M_Constructed(Value);
  
  *this = val; // operator= will take care of delete if needed

  return *this;
}

Value& Value::operator=(void *v)
{
  if (mType == eValueUnknown
  ||  mType == eValueVoidPtr)
  {
    mType = eValueVoidPtr;

    mpValVoid = v;

    return *this;
  }

  Value val(v); M_Constructed(Value);

  *this = val; // operator= will take care of delete if needed

  return *this;
}

bool Value::operator==(const Value& v) const
{
  // For a successful compare the two types need to be
  // compatible, currently the following are compatible:
  // - Bit/unsigned/int
  // - Char*/ConstChar*/String
  // Also, for now, we assume Bits hold positive numbers only.

  switch (mType) {
    case eValueChar:
      switch (v.mType) {
        case eValueChar:
          if (mValChar == v.mValChar) return TRUE;
          else                        return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueCharPtr:
      switch (v.mType) {
        case eValueCharPtr:
          if (!strcmp(mpValChar, v.mpValChar)) return TRUE;
          else                                 return FALSE;
          break;
        case eValueConstCharConstPtr:
          if (!strcmp(mpValChar, v.mpValConstCharConst)) return TRUE;
          else                                           return FALSE;
          break;
        case eValueString:
          if (*v.mpValString == mpValChar) return TRUE;
          else                             return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueConstCharConstPtr:
      switch (v.mType) {
        case eValueCharPtr:
          if (!strcmp(mpValConstCharConst, v.mpValChar)) return TRUE;
          else                                           return FALSE;
          break;
        case eValueConstCharConstPtr:
          // If the const pointers pointing to const
          // string are equal, then the string must also.
          if (mpValConstCharConst == v.mpValConstCharConst) return TRUE;
          else                                              return FALSE;
          break;
        case eValueString:
          if (*v.mpValString == mpValConstCharConst) return TRUE;
          else                                       return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueString:
      switch (v.mType) {
        case eValueCharPtr:
          if (*mpValString == mpValChar) return TRUE;
          else                           return FALSE;
          break;
        case eValueConstCharConstPtr:
          if (*mpValString == mpValConstCharConst) return TRUE;
          else                                     return FALSE;
          break;
        case eValueString:
          if (*mpValString == *v.mpValString) return TRUE;
          else                                return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueBit:
      switch (v.mType) {
        case eValueUnsigned:
          if ((unsigned)*mpValBit == v.mValUnsigned) return TRUE;
          else                                       return FALSE;
          break;
        case eValueInt:
          if (v.mValInt < 0) return FALSE;
          else {
            if ((unsigned)*mpValBit == v.mValInt) return TRUE;
            else                                  return FALSE;
          }
          break;
        case eValueBit:
          if (*mpValBit == *v.mpValBit) return TRUE;
          else                          return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueUnsigned:
      switch (v.mType) {
        case eValueUnsigned:
          if (mValUnsigned == v.mValUnsigned) return TRUE;
          else                                return FALSE;
          break;
        case eValueInt:
          if (v.mValInt < 0) return FALSE;
          else {
            if (mValUnsigned == v.mValInt) return TRUE;
            else                           return FALSE;
          }
          break;
        case eValueBit:
          if (mValUnsigned == (unsigned)*mpValBit) return TRUE;
          else                                     return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueInt:
      switch (v.mType) {
        case eValueUnsigned:
          if (mValInt < 0) return FALSE;
          else {
            if (mValInt == v.mValUnsigned) return TRUE;
            else                           return FALSE;
          }
          break;
        case eValueInt:
          if (mValInt == v.mValInt) return TRUE;
          else                      return FALSE;
          break;
        case eValueBit:
          if (mValInt < 0) return FALSE;
          else {
            if ((unsigned)*v.mpValBit == mValInt) return TRUE;
            else                                  return FALSE;
          }
          break;
        default: assert(0);
      }
      break;
    case eValueVoidPtr:
      if (mpValVoid == v.mpValVoid) return TRUE;
      else                          return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator!=(const Value& v) const
{
  if (*this == v) return FALSE;
  else            return TRUE;
}

bool Value::operator>(const Value& v) const
{
  // For a successful compare the two types need to be
  // compatible, currently the following are compatible:
  // - Bit/unsigned/int
  // - Char*/ConstChar*/String
  // Also, for now, we assume Bits hold positive numbers only.

  switch (mType) {
    case eValueChar:
      switch (v.mType) {
        case eValueChar:
          if (mValChar > v.mValChar) return TRUE;
          else                       return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueCharPtr:
      switch (v.mType) {
        case eValueCharPtr:
          if (strcmp(mpValChar, v.mpValChar) > 0) return TRUE;
          else                                    return FALSE;
          break;
        case eValueConstCharConstPtr:
          if (strcmp(mpValChar, v.mpValConstCharConst) > 0) return TRUE;
          else                                              return FALSE;
          break;
        case eValueString:
          if (strcmp(mpValChar, (const char*)*v.mpValString) > 0) return TRUE;
          else                                              return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueConstCharConstPtr:
      switch (v.mType) {
        case eValueCharPtr:
          if (strcmp(mpValConstCharConst, v.mpValChar) > 0) return TRUE;
          else                                              return FALSE;
          break;
        case eValueConstCharConstPtr:
          // If the const pointers pointing to const
          // string are equal, then the string must also.
          if (strcmp(mpValConstCharConst, v.mpValConstCharConst) > 0) return TRUE;
          else                                                        return FALSE;
          break;
        case eValueString:
          if (strcmp((const char*)*v.mpValString, mpValConstCharConst) > 0) return TRUE;
          else                                                        return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueString:
      switch (v.mType) {
        case eValueCharPtr:
          if (strcmp((const char*)*mpValString, mpValChar) > 0) return TRUE;
          else                                            return FALSE;
          break;
        case eValueConstCharConstPtr:
          if (strcmp((const char*)*mpValString, mpValConstCharConst) > 0) return TRUE;
          else                                                     return FALSE;
          break;
        case eValueString:
          if (strcmp((const char*)*mpValString, (const char*)*v.mpValString) > 0) return TRUE;
          else                                                        return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueBit:
      switch (v.mType) {
        case eValueUnsigned:
          if ((unsigned)*mpValBit > v.mValUnsigned) return TRUE;
          else                                      return FALSE;
          break;
        case eValueInt:
          if (v.mValInt < 0) return TRUE;
          else {
            if ((unsigned)*mpValBit > v.mValInt) return TRUE;
            else                                 return FALSE;
          }
          break;
        case eValueBit:
          if (*mpValBit > *v.mpValBit) return TRUE;
          else                         return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueUnsigned:
      switch (v.mType) {
        case eValueUnsigned:
          if (mValUnsigned > v.mValUnsigned) return TRUE;
          else                               return FALSE;
          break;
        case eValueInt:
          if (v.mValInt < 0) return TRUE;
          else {
            if (mValUnsigned > v.mValInt) return TRUE;
            else                          return FALSE;
          }
          break;
        case eValueBit:
          if (mValUnsigned > (unsigned)*mpValBit) return TRUE;
          else                                    return FALSE;
          break;
        default: assert(0);
      }
      break;
    case eValueInt:
      switch (v.mType) {
        case eValueUnsigned:
          if (mValInt < 0) return FALSE;
          else {
            if (mValInt > v.mValUnsigned) return TRUE;
            else                          return FALSE;
          }
          break;
        case eValueInt:
          if (mValInt > v.mValInt) return TRUE;
          else                     return FALSE;
          break;
        case eValueBit:
          if (mValInt < 0) return FALSE;
          else {
            if ((unsigned)*v.mpValBit < mValInt) return TRUE;
            else                                 return FALSE;
          }
          break;
        default: assert(0);
      }
      break;
    case eValueVoidPtr:
      if (mpValVoid == v.mpValVoid) return TRUE;
      else                          return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator<(const Value& v) const
{
  // Simply make use of operator '>' and '=='.
  // Need to be careful for Bit comparison, cannot split the < into
  // two separate comparisons because Bit will reset msb, lsb,
  // this applies whether Bit type is lhs or rhs.

  if (mType == eValueBit) {
    switch (v.mType) {
      case eValueBit:
        if (*mpValBit < *v.mpValBit) return TRUE;
        break;
      case eValueUnsigned:
        if ((unsigned)*mpValBit < v.mValUnsigned) return TRUE;
        break;
      case eValueInt:
        if      ( v.mValInt < 0)                  return FALSE;
        else if ((unsigned)*mpValBit < v.mValInt) return TRUE;
        break;
    }
  }
  else if (v.mType == eValueBit) {
    switch (mType) {
      case eValueUnsigned:
        if (mValUnsigned < (unsigned)*v.mpValBit) return TRUE;
        break;
      case eValueInt:
        if      (mValInt < 0)                     return TRUE;
        else if (mValInt < (unsigned)*v.mpValBit) return TRUE;
        break;
    }
  }
  else {
    return (! ((*this > v) || (*this == v)));
  }

  assert(0); // should not reach here
}

bool Value::operator<=(const Value& v) const
{
  // Simply invoke > operator.
  // Need to be careful for Bit comparison, cannot split the <= into
  // two separate comparisons because Bit will reset msb, lsb,
  // this applies whether Bit type is lhs or rhs.

  if (mType == eValueBit) {
    switch (v.mType) {
      case eValueBit:
        if (*mpValBit <= *v.mpValBit) return TRUE;
        break;
      case eValueUnsigned:
        if ((unsigned)*mpValBit <= v.mValUnsigned) return TRUE;
        break;
      case eValueInt:
        if      ( v.mValInt < 0)                   return FALSE;
        else if ((unsigned)*mpValBit <= v.mValInt) return TRUE;
        break;
    }
  }
  else if (v.mType == eValueBit) {
    switch (mType) {
      case eValueUnsigned:
        if (mValUnsigned <= (unsigned)*v.mpValBit) return TRUE;
        break;
      case eValueInt:
        if      (mValInt < 0)                      return TRUE;
        else if (mValInt <= (unsigned)*v.mpValBit) return TRUE;
        break;
    }
  }
  else {
    return !(*this > v);
  }

  assert(0); // should not reach here
}

bool Value::operator>=(const Value& v) const
{
  // Need to be careful for Bit comparison, cannot split the <= into
  // two separate comparisons because Bit will reset msb, lsb,
  // this applies whether Bit type is lhs or rhs.

  if (mType == eValueBit) {
    switch (v.mType) {
      case eValueBit:
        if (*mpValBit >= *v.mpValBit) return TRUE;
      break;
      case eValueUnsigned:
        if ((unsigned)*mpValBit >= v.mValUnsigned) return TRUE;
      case eValueInt:
        if (v.mValInt < 0)                         return TRUE;
        else if ((unsigned)*mpValBit >= v.mValInt) return TRUE;
    }
  }
  else if (v.mType == eValueBit) {
    switch (mType) {
      case eValueUnsigned:
        if (mValUnsigned >= (unsigned)*v.mpValBit) return TRUE;
        break;
      case eValueInt:
        if      (mValInt < 0)                      return FALSE;
        else if (mValInt >= (unsigned)*v.mpValBit) return TRUE;
        break;
    }
  }
  else {
    return (*this>v || *this==v);
  }

  assert(0); // should not reach here
}

bool Value::operator==(int v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return FALSE;
      else {
        if ((unsigned)*mpValBit == v) return TRUE;
        else                          return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return FALSE;
      else {
        if (mValUnsigned == v) return TRUE;
        else                   return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt == v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator!=(int v) const
{
  return !(*this == v);
}

bool Value::operator>(int v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return TRUE;
      else {
        if ((unsigned)*mpValBit > v) return TRUE;
        else                         return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return TRUE;
      else {
        if (mValUnsigned > v) return TRUE;
        else                  return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt > v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<(int v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return FALSE;
      else {
        if ((unsigned)*mpValBit < v) return TRUE;
        else                         return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return FALSE;
      else {
        if (mValUnsigned < v) return TRUE;
        else                  return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt < v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<=(int v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return FALSE;
      else {
        if ((unsigned)*mpValBit <= v) return TRUE;
        else                          return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return FALSE;
      else {
        if (mValUnsigned <= v) return TRUE;
        else                   return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt <= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator>=(int v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return TRUE;
      else {
        if ((unsigned)*mpValBit >= v) return TRUE;
        else                          return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return TRUE;
      else {
        if (mValUnsigned >= v) return TRUE;
        else                   return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt >= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator==(short v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return FALSE;
      else {
        if ((unsigned)*mpValBit == v) return TRUE;
        else                          return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return FALSE;
      else {
        if (mValUnsigned == v) return TRUE;
        else                   return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt == v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator!=(short v) const
{
  return !(*this == v);
}

bool Value::operator>(short v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return TRUE;
      else {
        if ((unsigned)*mpValBit > v) return TRUE;
        else                         return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return TRUE;
      else {
        if (mValUnsigned > v) return TRUE;
        else                  return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt > v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<(short v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return FALSE;
      else {
        if ((unsigned)*mpValBit < v) return TRUE;
        else                         return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return FALSE;
      else {
        if (mValUnsigned < v) return TRUE;
        else                  return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt < v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<=(short v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return FALSE;
      else {
        if ((unsigned)*mpValBit <= v) return TRUE;
        else                          return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return FALSE;
      else {
        if (mValUnsigned <= v) return TRUE;
        else                   return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt <= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator>=(short v) const
{
  switch (mType) {
    case eValueBit:
      if (v < 0) return TRUE;
      else {
        if ((unsigned)*mpValBit >= v) return TRUE;
        else                          return FALSE;
      }
      break;
    case eValueUnsigned:
      if (v < 0) return TRUE;
      else {
        if (mValUnsigned >= v) return TRUE;
        else                   return FALSE;
      }
      break;
    case eValueInt:
      if (mValInt >= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator==(unsigned short v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit == v) return TRUE;
      else                          return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned == v) return TRUE;
      else                   return FALSE;
      break;
    case eValueInt:
      if (mValInt < 0)  return FALSE;
      if (mValInt == v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator!=(unsigned short v) const
{
  return !(*this == v);
}

bool Value::operator>(unsigned short v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit > v) return TRUE;
      else                         return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned > v) return TRUE;
      else                  return FALSE;
      break;
    case eValueInt:
      if (mValInt < 0) return TRUE;
      if (mValInt > v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<(unsigned short v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit < v) return TRUE;
      else                         return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned < v) return TRUE;
      else                  return FALSE;
      break;
    case eValueInt:
      if (mValInt < 0) return TRUE;
      if (mValInt < v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<=(unsigned short v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit <= v) return TRUE;
      else                          return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned <= v) return TRUE;
      else                   return FALSE;
      break;
    case eValueInt:
      if (mValInt <  0) return TRUE;
      if (mValInt <= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator>=(unsigned short v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit >= v) return TRUE;
      else                          return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned >= v) return TRUE;
      else                   return FALSE;
      break;
    case eValueInt:
      if (mValInt <  0) return FALSE;
      if (mValInt >= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator==(unsigned v) const
{
  switch (mType) {
  case eValueBit:
    if ((unsigned)*mpValBit == v) return TRUE;
    else                          return FALSE;
  case eValueUnsigned:
    if (mValUnsigned == v) return TRUE;
    else                   return FALSE;
  case eValueInt:
    if (mValInt < 0) return FALSE;
    else {
      if (mValInt == v) return TRUE;
      else              return FALSE;
    }
  default: assert(0);
  }

  return FALSE;
}

bool Value::operator!=(unsigned v) const
{
  return !(*this == v);
}

bool Value::operator>(unsigned v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit > v) return TRUE;
      else                         return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned > v) return TRUE;
      else                  return FALSE;
      break;
    case eValueInt:
      if (mValInt < 0) return TRUE;
      if (mValInt > v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<(unsigned v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit < v) return TRUE;
      else                         return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned < v) return TRUE;
      else                  return FALSE;
      break;
    case eValueInt:
      if (mValInt < 0) return TRUE;
      if (mValInt < v) return TRUE;
      else             return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator<=(unsigned v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit <= v) return TRUE;
      else                          return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned <= v) return TRUE;
      else                   return FALSE;
      break;
    case eValueInt:
      if (mValInt <  0) return TRUE;
      if (mValInt <= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator>=(unsigned v) const
{
  switch (mType) {
    case eValueBit:
      if ((unsigned)*mpValBit >= v) return TRUE;
      else                          return FALSE;
      break;
    case eValueUnsigned:
      if (mValUnsigned >= v) return TRUE;
      else                   return FALSE;
      break;
    case eValueInt:
      if (mValInt <  0) return FALSE;
      if (mValInt >= v) return TRUE;
      else              return FALSE;
      break;
    default:
      assert(0);
  }

  assert(0);
}

bool Value::operator==(char* v) const
{
  Value val(v); M_Constructed(Value);

  return (*this == val);
}

bool Value::operator!=(char* v) const
{
  Value val(v); M_Constructed(Value);

  return (*this != val);
}

bool Value::operator>(char* v) const
{
  Value val(v); M_Constructed(Value);

  return (*this > val);
}

bool Value::operator<(char* v) const
{
  Value val(v); M_Constructed(Value);

  return (*this < val);
}

bool Value::operator<=(char* v) const
{
  Value val(v); M_Constructed(Value);

  return (*this <= val);
}

bool Value::operator>=(char* v) const
{
  Value val(v); M_Constructed(Value);

  return (*this >= val);
}

bool Value::operator==(const char* v) const
{
  Value val((const char*)v); M_Constructed(Value);

  return (*this == val);
}

bool Value::operator!=(const char* v) const
{
  Value val((const char*)v); M_Constructed(Value);

  return (*this != val);
}

bool Value::operator>(const char* v) const
{
  Value val((const char*)v); M_Constructed(Value);

  return (*this > val);
}

bool Value::operator<(const char* v) const
{
  Value val((const char*)v); M_Constructed(Value);

  return (*this < val);
}

bool Value::operator<=(const char* v) const
{
  Value val((const char*)v); M_Constructed(Value);

  return (*this <= val);
}

bool Value::operator>=(const char* v) const
{
  Value val((const char*)v); M_Constructed(Value);

  return (*this >= val);
}

// TBD: Add the above comparisons for 'const char* const' also.

bool Value::operator==(Bit& b) const
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueBit:
      if (*mpValBit == b) return TRUE;
      else                return FALSE;
    case eValueUnsigned:
      if (mValUnsigned == (unsigned int)b) return TRUE;
      else                                 return FALSE;
    case eValueInt:
      if (mValInt < 0) return FALSE;
      else {
        if (mValInt == (unsigned int)b) return TRUE;
        else                            return FALSE;
      }
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator!=(Bit& b) const
{
  if (*this == b) return FALSE;
  else            return TRUE;
}

bool Value::operator>(Bit& b) const
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueBit:
      if (*mpValBit > b) return TRUE;
      else               return FALSE;
    case eValueUnsigned:
      if (mValUnsigned > (unsigned int)b) return TRUE;
      else                                return FALSE;
    case eValueInt:
      if (mValInt < 0) return FALSE;
      else {
        if (mValInt > (unsigned int)b) return TRUE;
        else                           return FALSE;
      }
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator<(Bit& b) const
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueBit:
      if (*mpValBit < b) return TRUE;
      else               return FALSE;
    case eValueUnsigned:
      if (mValUnsigned < (unsigned int)b) return TRUE;
      else                                return FALSE;
    case eValueInt:
      if (mValInt < 0) return TRUE;
      else {
        if (mValInt < (unsigned int)b) return TRUE;
        else                           return FALSE;
      }
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator>=(Bit& b) const
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueBit:
      if (*mpValBit >= b) return TRUE;
      else                  return FALSE;
    case eValueUnsigned:
      if (mValUnsigned >= (unsigned int)b) return TRUE;
      else                                 return FALSE;
    case eValueInt:
      if (mValInt < 0) return FALSE;
      else {
        if (mValInt >= (unsigned int)b) return TRUE;
        else                            return FALSE;
      }
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator<=(Bit& b) const
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueBit:
      if (*mpValBit <= b) return TRUE;
      else                  return FALSE;
    case eValueUnsigned:
      if (mValUnsigned <= (unsigned int)b) return TRUE;
      else                                 return FALSE;
    case eValueInt:
      if (mValInt < 0) return TRUE;
      else {
        if (mValInt <= (unsigned int)b) return TRUE;
        else                            return FALSE;
      }
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator==(void *v) const
{
  switch (mType) {
  case eValueCharPtr:
    return mpValChar == v;
    break;
  case eValueConstCharConstPtr:
    return mpValConstCharConst == v;
    break;
  case eValueVoidPtr:
    return mpValVoid == v;
    break;
  default:
    assert(0);
  }

  assert(0);
}

bool Value::operator!=(void *v) const
{
  return !(this == v);
}

bool Value::operator==(bool v) const
{
  switch (mType) {
    case eValueUnsigned:
      return  mValUnsigned == v;
    case eValueInt:
      return  mValInt == v;
    case eValueBit: {
      Bit b;
      M_Constructed(Bit);
      b = v;

      return (*mpValBit == b);
    }
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

bool Value::operator!=(bool v) const
{
  if (*this == v) return FALSE;
  else            return TRUE;
}

bool Value::operator!() const
{
  switch (mType) {
    case eValueUnknown:
      assert(0);
      return FALSE;
    case eValueBit: {
      Bit b(*mpValBit);
      M_Constructed(Bit);
      b = 0;
      return *mpValBit==b;
    }
    case eValueChar:
      return mValChar == '\0';
    case eValueCharPtr:
      return mpValChar == 0;
    case eValueConstCharConstPtr:
      return mpValConstCharConst == 0;
    case eValueVoidPtr:
      return mpValVoid == 0;
    case eValueString:
      return *mpValString == "";
    case eValueUnsigned:
      return mValUnsigned == 0;
    case eValueInt:
      return mValInt == 0;
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

Condition& Value::operator==(const Attribute& attr) const
{
  Condition* c = new Condition(*this, TRUE);
  assert(c);

  return *c == attr;
}

Condition& Value::operator!=(const Attribute& attr) const
{
  Condition* c = new Condition(*this, TRUE);
  assert(c);

  return *c != attr;
}

Condition& Value::operator>(const Attribute& attr) const
{
  Condition* c = new Condition(*this, TRUE);
  assert(c);

  return *c > attr;
}

Condition& Value::operator<(const Attribute& attr) const
{
  Condition* c = new Condition(*this, TRUE);
  assert(c);

  return *c < attr;
}

Condition& Value::operator<=(const Attribute& attr) const
{
  Condition* c = new Condition(*this, TRUE);
  assert(c);

  return *c <= attr;
}

Condition& Value::operator>=(const Attribute& attr) const
{
  Condition* c = new Condition(*this, TRUE);
  assert(c);

  return *c >= attr;
}

void Value::ValueCopyGet(Bit& bit)
{
  assert (mType == eValueBit);

  bit = *mpValBit;
}

eCompare Value::Compare(Value& val)
{
  if (*this < val) return eCompareLess;
  if (*this > val) return eCompareGreater;

  return eCompareEqual;
}

Value::operator short()
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueUnsigned:
      if (mValUnsigned > SHRT_MAX) assert(0);
      else                         return (mValUnsigned&SHRT_MAX);
    case eValueInt:
      if      (mValInt > SHRT_MAX) assert(0);
      else if (mValInt < SHRT_MIN) assert(0);
      else                         return (mValInt&SHRT_MAX);
    case eValueBit:
      return (short)(*mpValBit);
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

Value::operator unsigned short()
{
  unsigned short s;

  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueUnsigned:
      if (mValUnsigned > USHRT_MAX) assert(0);
      else                          return s = mValUnsigned&USHRT_MAX;
    case eValueInt:
      if      (mValInt > USHRT_MAX) assert(0);
      else if (mValInt < 0)         assert(0);
      else                          return s = mValInt&USHRT_MAX;
    case eValueBit:
      return (unsigned short)(*mpValBit);
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

Value::operator int()
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueUnsigned:
      if (mValUnsigned > INT_MAX) assert(0);
      else                        return (mValUnsigned&INT_MAX);
    case eValueInt:
      return mValInt;
    case eValueBit:
      return (int)(*mpValBit);
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

Value::operator unsigned int()
{
  assert(mType==eValueBit || mType==eValueUnsigned || mType==eValueInt);

  switch (mType) {
    case eValueUnsigned:
      return mValUnsigned&UINT_MAX;
    case eValueInt:
      if (mValInt < 0) assert(0);
      else             return mValInt&UINT_MAX;
    case eValueBit:
      return (unsigned int)(*mpValBit);
    default:
      assert(0);
  }

  assert(0); // should not reach here
}

Value::operator char()
{
  char c;

  assert(   mType==eValueConstCharConstPtr || mType==eValueChar
         || mType==eValueCharPtr           || mType==eValueString);

  switch (mType) {
    case eValueChar:
      return c = mValChar;
    case eValueCharPtr:
      if (mpValChar!=0) return c = mpValChar[0];
      else              return c = '\0';
    case eValueConstCharConstPtr:
      if (mpValConstCharConst!=0) return c = mpValConstCharConst[0];
      else                        return c = '\0';
    case eValueString:
      return c=mpValString->Getc(0);
    default:
      assert(0);
      return c='\0';
  }

  assert(0); // should not reach here
}

Value::operator const char*()
{
  assert(   mType == eValueCharPtr
         || mType == eValueConstCharConstPtr
         || mType == eValueString);

  switch (mType) {
    case eValueConstCharConstPtr:
      return (const char*)mpValConstCharConst;
    case eValueString:
      // This is not clean but a compromise solution.
      // Problem is when the String is deleted the pointer
      // returned here becomes invalid.
      return (const char*)mpValString;
    case eValueCharPtr:
      return (const char*)mpValChar;
    default: assert(0);
  }

  assert(0); // should not reach here
}

Value::operator char*()
{
  assert(mType==eValueCharPtr);

  if (mType == eValueCharPtr) return mpValChar;

  return (char*)"";
}

Value::operator Bit()
{
  assert(mType==eValueBit);

  return *mpValBit;
}

void Value::SetMsbLsb(short msb, short lsb) const
{
  assert(mType == eValueBit);

  (*mpValBit).SetMsbLsb(msb, lsb);
}

char* Value::Save(char buf[MAX_RECORD_LINE_LENGTH+1]) const
{
  switch (mType) {
    case eValueChar:
      buf[0] = mValChar; buf[1] = '\0';
      break;
    case eValueCharPtr:
      snprintf(buf, MAX_RECORD_LINE_LENGTH, "%s", mpValChar);
      break;
    case eValueConstCharConstPtr:
      snprintf(buf, MAX_RECORD_LINE_LENGTH, "%s", mpValConstCharConst);
      break;
    case eValueString:
      snprintf(buf, MAX_RECORD_LINE_LENGTH, "%s", (const char*)(*mpValString));
      break;
    case eValueBit: {
      char bit_buf [MAX_RECORD_LINE_LENGTH+1];

      mpValBit->Save(bit_buf);
      snprintf(buf, MAX_RECORD_LINE_LENGTH, "%s", bit_buf);
      break;
    }
    case eValueUnsigned:
      snprintf(buf, MAX_RECORD_LINE_LENGTH, "%u", mValUnsigned);
      break;
    case eValueInt:
      snprintf(buf, MAX_RECORD_LINE_LENGTH, "%u", mValInt);
      break;
    case eValueVoidPtr:
      snprintf(buf, MAX_RECORD_LINE_LENGTH, "%p", mpValVoid); // fix for 64 bit
      break;
    default:
      strcpy(buf, "???");
  }

  return buf;
}

bool Value::Initialize(const char* buf, short bit_width)
{
  int        ret;
  char       str [MAX_RECORD_LINE_LENGTH+1];
  bool       status;
  unsigned   u;
  int        ii;

  if (buf[0] == '\0') return status = FALSE;

  status = TRUE;

  switch (mType) {
    eValueUnknown:
      status = FALSE;
      break;
    eValueBit:
      if (mNewCalled) { delete mpValBit; mpValBit=0; }

      if (bit_width) {
        mpValBit = new Bit(bit_width);
        M_Constructed(Bit);

        if (mpValBit) {
          const char* format = "";
          if (!mpValBit->Initialize(buf, format)) status = FALSE;
        }
      }
      else {
        status = FALSE;
      }
      break;
    eValueChar:
      mValChar = buf[0];
      break;
    eValueCharPtr:
      if (mNewCalled) { M_UpdDestructCountN(eType_Char, strlen(mpValChar)+1); delete [] mpValChar; mpValChar=0; }

      mpValChar = new char [strlen(buf)+1]; M_UpdConstructCountN(eType_Char, strlen(buf)+1);
      if (mpValChar) {
        strcpy(mpValChar, buf);
        mNewCalled = TRUE;
      }
      break;
    eValueConstCharConstPtr:
      status = FALSE; // cannot be initialized from a buffer
      break;
    eValueVoidPtr:
      status = FALSE; // no sense storing pointers to/from buf/file
      break;
    eValueString:
      if (mNewCalled) { delete mpValString; mpValString=0; }

      mpValString = new String(buf);
      if (mpValString) mNewCalled = TRUE;
      break;
    eValueUnsigned:
      ret = sscanf(buf, "%u", &u);

      if (ret) mValUnsigned = u;
      else     status       = FALSE;

      break;
    eValueInt:
      ret = sscanf(buf, "%d", &ii);

      if (ret) mValInt = ii;
      else     status  = FALSE;

      break;
    default:
      status = FALSE;
  }

  return status;
}

void Value::ValueFree()
{
  switch (mType) {
    case eValueChar:
    case eValueUnsigned:
    case eValueInt:
    case eValueUnknown:
      // nothing to free for built-in types.
      break;
    case eValueCharPtr:
      if (mNewCalled) {
        if (mType == eValueCharPtr) { M_UpdDestructCountN(eType_Char, strlen(mpValChar)+1); delete [] mpValChar; mpValChar=0; }

        mNewCalled = FALSE;
      }
      break;
    case eValueConstCharConstPtr:
      break;
    case eValueString:
      if (mNewCalled) {
        delete mpValString;

        mNewCalled = FALSE;
      }
      break;
    case eValueBit:
      if (mNewCalled) {
        delete mpValBit;

        mNewCalled = FALSE;
      }
      break;
    case eValueVoidPtr:
      break; // we dont delete void pointers, defer to creator/allocator
    default:
      assert(0);
  }
}

void Value::ValueTypeSet(eValueType type)
{
  if (mType == eValueUnknown) mType = type;
}

void Value::SetToMax()
{
  switch (mType) {
    case eValueUnsigned:
      mValUnsigned = UINT_MAX;
      break;
    case eValueInt:
      mValInt = INT_MAX;
      break;
    case eValueBit:
      mpValBit->BitSetAll();
      break;
    case eValueCharPtr:
    case eValueConstCharConstPtr:
    case eValueChar:
    case eValueString:
    case eValueVoidPtr:
      break; // do nothing, maybe add warning in future
    default:
      assert(0);
  }
}

void Value::SetToMin()
{
  switch (mType) {
    case eValueUnsigned:
      mValUnsigned = 0;
      break;
    case eValueInt:
      mValInt = 0;
      break;
    case eValueBit:
      mpValBit->BitClearAll();
      break;
    case eValueCharPtr:
    case eValueConstCharConstPtr:
    case eValueChar:
    case eValueString:
    case eValueVoidPtr:
      break; // do nothing, maybe add warning in future
    default:
      assert(0);
  }
}

void Value::Print(FILE* fp)
{
  char buf [MAX_RECORD_LINE_LENGTH+1];

  if (fp == 0) fp = stdout;

  fprintf(fp, "%s", Save(buf));
}

Value::operator Value*() const
{
  assert(0);
}

// End
