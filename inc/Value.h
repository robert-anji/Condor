// $RCSfile: Value.h,v $
// $Revision: 1.8 $

#ifndef ValueClass
#define ValueClass

#include <stdio.h>

#include "Attribute.h"
#include "DBDefines.h"
#include "Global.h"
#include "String.h"
#include "Enum.h"

class Bit;

class Value
{
  friend class Table;

public:
  Value();
  Value(eValueType      type);
  Value(const Value    &v);
  Value(short           v);
  Value(unsigned short  v);
  Value(int             v);
  Value(unsigned int    v);
  Value(char            v);
  Value(const String    *v);
  Value(const String   &v);
  Value(const char     *v);
  Value(const char* &   v);
  Value(const Bit      *v);
  Value(const Bit      &v);
  Value(void           *v);

  unsigned Hash();

  ~Value();

  Value& operator++();     // prefix
  Value& operator++(int);  // postfix
  Value& operator--();
  Value& operator--(int);
  Value& operator+ (const Value& val);
  Value& operator+=(const Value& val);
  Value& operator- (const Value& val);
  Value& operator-=(const Value& val);
  Value& operator* (const Value& val);
  Value& operator/ (const Value& val);

//Value& operator+ (short s);
//Value& operator+=(short s);
//Value& operator- (short s);
//Value& operator-=(short s);
//Value& operator* (short s);
//Value& operator/ (short s);

  Value& operator+ (unsigned u);
  Value& operator+=(unsigned u);
  Value& operator- (unsigned u);
  Value& operator-=(unsigned u);
  Value& operator* (unsigned u);
  Value& operator/ (unsigned u);

//Value& operator=(Value  v);
  Value& operator=(const Value& v);
  Value& operator=(short);
  Value& operator=(unsigned short);
  Value& operator=(int);
  Value& operator=(unsigned int);
  Value& operator=(char);
  Value& operator=(const char*);
  Value& operator=(Bit&);
  Value& operator=(void*);

  bool operator==(const Value& v) const;
  bool operator!=(const Value& v) const;
  bool operator> (const Value& v) const;
  bool operator< (const Value& v) const;
  bool operator<=(const Value& v) const;
  bool operator>=(const Value& v) const;

  bool operator==(short v) const;
  bool operator!=(short v) const;
  bool operator> (short v) const;
  bool operator< (short v) const;
  bool operator<=(short v) const;
  bool operator>=(short v) const;

  bool operator==(unsigned v) const;
  bool operator!=(unsigned v) const;
  bool operator> (unsigned v) const;
  bool operator< (unsigned v) const;
  bool operator<=(unsigned v) const;
  bool operator>=(unsigned v) const;

  bool operator==(int v) const;
  bool operator!=(int v) const;
  bool operator> (int v) const;
  bool operator< (int v) const;
  bool operator<=(int v) const;
  bool operator>=(int v) const;

  bool operator==(unsigned short v) const;
  bool operator!=(unsigned short v) const;
  bool operator> (unsigned short v) const;
  bool operator< (unsigned short v) const;
  bool operator<=(unsigned short v) const;
  bool operator>=(unsigned short v) const;

  bool operator==(char* v) const;
  bool operator!=(char* v) const;
  bool operator> (char* v) const;
  bool operator< (char* v) const;
  bool operator<=(char* v) const;
  bool operator>=(char* v) const;

  bool operator==(const char* v) const;
  bool operator!=(const char* v) const;
  bool operator> (const char* v) const;
  bool operator< (const char* v) const;
  bool operator<=(const char* v) const;
  bool operator>=(const char* v) const;

  bool operator==(Bit& b) const;
  bool operator!=(Bit& b) const;
  bool operator> (Bit& b) const;
  bool operator< (Bit& b) const;
  bool operator<=(Bit& b) const;
  bool operator>=(Bit& b) const;

  Condition& operator==(const Attribute& attr) const;
  Condition& operator!=(const Attribute& attr) const;
  Condition& operator> (const Attribute& attr) const;
  Condition& operator< (const Attribute& attr) const;
  Condition& operator<=(const Attribute& attr) const;
  Condition& operator>=(const Attribute& attr) const;

  bool operator==(void *v) const;
  bool operator!=(void *v) const;

  bool operator==(bool v) const;
  bool operator!=(bool v) const;

  bool operator!() const;

  // Get Bit value
  void ValueCopyGet(Bit& bit);

  // Compare() used by List<> to sort
  eCompare Compare(Value& val);

  // Cast value to another type.
  operator short();
  operator unsigned short();
  operator int();
  operator unsigned int();
  operator char();
  operator char*();
  operator const char*();
  operator Bit();

  // Need a function to set the msb, lsb for Bit value
  void        SetMsbLsb(short msb, short lsb) const;

  eValueType  ValueType() const { return mType; }

  operator void*()  { return (void*)mpValVoid; }
  operator Value*() const;

  // Utilities:
  void   ValueTypeSet(eValueType type);
  void   SetToMax();
  void   SetToMin();
  void   Print(FILE* fp=0);
  char*  Save(char buf[MAX_RECORD_LINE_LENGTH+1]) const;

private:
  // Functions:
  void   ValueFree();
  bool   Initialize(const char* line, short bit_width=0);

  // Data:
  eValueType         mType;
  bool               mNewCalled;

  union {
    char         mValChar;
    char*        mpValChar;
    const char*  mpValConstCharConst;
    void*        mpValVoid;
    unsigned     mValUnsigned;
    int          mValInt;
    String*      mpValString;
    Bit*         mpValBit;
  };
};

#endif

// End
