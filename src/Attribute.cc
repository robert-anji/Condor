// $RCSfile: Attribute.cc,v $
// $Revision: 1.6 $

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Attribute.h"
//
#include "Condition.h"
#include "Database.h"
#include "Enum.h"
#include "Value.h"

M_ExternConstructDestructCountInt;

Attribute::Attribute() :
  mValueType(eValueUnknown),
  mBitWidth(0),
  mPosition(0),
  mpAttrNullValue(0),
  mpTable(0),
  mpView(0)
{
  M_UpdConstructCount(eType_Attribute);
}

Attribute::Attribute(const char* nm) :
  mName(nm),
  mValueType(eValueUnsigned),//TBD: make unknown?
  mBitWidth(0),
  mPosition(0),
  mpAttrNullValue(0),
  mpTable(0),
  mpView(0)
{
  M_UpdConstructCount(eType_Attribute);
}

Attribute::Attribute(const char* nm, eValueType typ, Value *null_val, short bit_width) :
  mName(nm),
  mValueType(typ),
  mBitWidth(bit_width),
  mPosition(0),
  mpAttrNullValue(0),
  mpTable(0),
  mpView(0)
{
  if (mValueType == eValueBit) assert(mBitWidth);

  if (null_val) {
    mpAttrNullValue = new Value(*null_val); M_Constructed(Value);
    assert(mpAttrNullValue);
  }

  M_UpdConstructCount(eType_Attribute);
}

Attribute::Attribute(char* nm) :
  mName(nm),
  mValueType(eValueUnsigned),//TBD: Make unknown?
  mBitWidth(0),
  mPosition(0),
  mpAttrNullValue(0),
  mpTable(0),
  mpView(0)
{
  assert(mValueType!=eValueBit); // bit needs separate constructor

  M_UpdConstructCount(eType_Attribute);
}

Attribute::Attribute(char* nm, eValueType typ) :
  mName(nm),
  mValueType(typ),
  mBitWidth(0),
  mPosition(0),
  mpAttrNullValue(0),
  mpTable(0),
  mpView(0)
{
  assert(mValueType != eValueBit); // Bit needs separate constructor

  M_UpdConstructCount(eType_Attribute);
}

Attribute::Attribute(Attribute& attr)
{
  mValueType  = attr.mValueType;
  mName       = attr.mName;
  mPosition   = attr.mPosition;
  mBitWidth   = attr.mBitWidth;
  mpTable     = attr.mpTable;

  *mpAttrNullValue = *(attr.mpAttrNullValue);

  M_UpdConstructCount(eType_Attribute);
}

Attribute::~Attribute()
{
  if (mpAttrNullValue) { delete mpAttrNullValue; mpAttrNullValue=0; }

  M_UpdDestructCount(eType_Attribute);
}

Condition& Attribute::operator<(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c < val;
}

Condition& Attribute::operator>(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c > val;
}

Condition& Attribute::operator>=(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >= val;
}

Condition& Attribute::operator<=(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <= val;
}

Condition& Attribute::operator==(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == val;
}

Condition& Attribute::operator!=(unsigned val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != val;
}

Condition& Attribute::operator<(int val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c < val;
}

Condition& Attribute::operator>(int val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c > val;
}

Condition& Attribute::operator>=(int val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >= val;
}

Condition& Attribute::operator<=(int val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <= val;
}

Condition& Attribute::operator==(int val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == val;
}

Condition& Attribute::operator!=(int val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != val;
}

Condition& Attribute::operator<(short val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c < val;
}

Condition& Attribute::operator>(short val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c > val;
}

Condition& Attribute::operator>=(short val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >= val;
}

Condition& Attribute::operator<=(short val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <= val;
}

Condition& Attribute::operator==(short val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == val;
}

Condition& Attribute::operator!=(short val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != val;
}

Condition& Attribute::operator<(char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <  val;
}

Condition& Attribute::operator>(char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >  val;
}

Condition& Attribute::operator>=(char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >= val;
}

Condition& Attribute::operator<=(char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <= val;
}

Condition& Attribute::operator==(char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == val;
}

Condition& Attribute::operator!=(char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != val;
}

Condition& Attribute::operator<(const char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <  val;
}

Condition& Attribute::operator>(const char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >  val;
}

Condition& Attribute::operator>=(const char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >= val;
}

Condition& Attribute::operator<=(const char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <= val;
}

Condition& Attribute::operator==(const char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == val;
}

Condition& Attribute::operator!=(const char* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != val;
}

Condition& Attribute::operator==(void* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == val;
}

Condition& Attribute::operator!=(void* val) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != val;
}

Condition& Attribute::operator!() const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return !(*c);
}

Condition& Attribute::operator==(Record* rec) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == rec;
}

Condition& Attribute::operator!=(Record* rec) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != rec;
}

Condition& Attribute::operator<(Attribute attr) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c < attr;
}

Condition& Attribute::operator>(Attribute attr) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c > attr;
}

Condition& Attribute::operator>=(Attribute attr) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c >= attr;
}

Condition& Attribute::operator<=(Attribute attr) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c <= attr;
}

Condition& Attribute::operator==(Attribute attr) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c == attr;
}

Condition& Attribute::operator!=(Attribute attr) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c != attr;
}

// Allow value pointers as RHS operands.
Condition& Attribute::operator<(Value* pval) const
{
  Condition* c = new Condition(this, TRUE);
  assert(c);

  return *c < pval;
}

const char* AttributeTypeToStr(eValueType type)
{
  switch (type) {
    case eValueUnknown:             return "Unknown";
    case eValueBit:                 return "Bit";
    case eValueChar:                return "Char";
    case eValueCharPtr:             return "CharPtr";
    case eValueConstCharConstPtr:   return "ConstCharPtr";
    case eValueVoidPtr:             return "VoidPtr";
    case eValueString:              return "String";
    case eValueUnsigned:            return "Unsigned";
    case eValueInt:                 return "Int";
    default:                        return "???";
  }
}

eValueType AttributeStrToType(const char* type_str)
{
  if (!strcmp(type_str, "Bit"         )) return eValueBit;
  if (!strcmp(type_str, "Char"        )) return eValueChar;
  if (!strcmp(type_str, "CharPtr"     )) return eValueCharPtr;
  if (!strcmp(type_str, "ConstCharPtr")) return eValueConstCharConstPtr;
  if (!strcmp(type_str, "VoidPtr"     )) return eValueVoidPtr;
  if (!strcmp(type_str, "String"      )) return eValueString;
  if (!strcmp(type_str, "Unsigned"    )) return eValueUnsigned;
  if (!strcmp(type_str, "Int"         )) return eValueInt;

  return eValueUnknown;
}

bool Attribute::operator>>(FILE* fp)
{
  if (fp==0) return FALSE;

  fprintf(fp, "%s,%s", (const char*)mName, Enum2Str(ValueTypeNV, mValueType));

  return TRUE;
}

bool Attribute::operator<<(FILE* fp)
{
  char typestr[256];
  char namestr[256];

  int ret = fscanf(fp, "%255[^,\n],%255[^;\n]", namestr, typestr);

  if (ret != 2)
  {
    fscanf(fp, "%*c");  // grab \n
    return FALSE;
  }

  typestr[255] = '\0';
  namestr[255] = '\0';

  mName = namestr;

  mValueType = (eValueType)Str2Enum(ValueTypeNV, typestr);

  return TRUE;
}

bool Attribute::AttributeIsSame(Attribute* attr)
{
  return (   mName           == attr->mName
          && mValueType      == attr->mValueType
          && mBitWidth       == attr->mBitWidth
          && mPosition       == attr->mPosition
          && mpAttrNullValue == attr->mpAttrNullValue
          && mpTable         == attr->mpTable);
}

// End
