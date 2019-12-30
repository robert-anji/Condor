// $RCSfile: Attribute.h,v $
// $Revision: 1.9 $

#ifndef AttributeClass
#define AttributeClass

#include <stdio.h>

#include "DBDefines.h"
#include "Global.h"
#include "String.h"

class Condition;
class Record;
class Table;
class Value;
class View;

eValueType  AttributeStrToType(const char* type_str);
const char* AttributeTypeToStr(eValueType type);

class Attribute
{
  friend class Table;
  friend class ViewJoin;

public:
  inline short PositionGet(void) const { return mPosition; }
  inline void  Name(FILE *fp)          { *this >> fp;      }
  inline const char* Name()      const { return (const char*)mName; }

  // Constructors:
  Attribute();
  Attribute(const char* nm);
  Attribute(const char* nm, eValueType typ,  Value *null_val=0, short bit_width=0);
  Attribute(char* nm);
  Attribute(char* nm, eValueType typ);
  Attribute(Attribute& attr);
  //
  ~Attribute();

  // Overloaded operators needed to convert Attributes into queries on-the-fly.
  #include "ConditionCreateOperators.h"

  Condition& operator==(Record* rec) const;
  Condition& operator!=(Record* rec) const;

  // Comparing attributes needed for Join's.
  Condition& operator< (Attribute attr) const;
  Condition& operator> (Attribute attr) const;
  Condition& operator>=(Attribute attr) const;
  Condition& operator<=(Attribute attr) const;
  Condition& operator==(Attribute attr) const;
  Condition& operator!=(Attribute attr) const;

  // Set view if attribute needs to be qualified for a specific view.
  Attribute& operator[](View* view) { mpView = view; }
  View*      ViewGet() const        { return mpView; }

  // Allow value pointers as RHS operands.
  Condition& operator< (Value* pval) const;

  void   PositionSet(short s) { mPosition = s;    }
  short  BitWidth() const     { return mBitWidth; }
  bool   AttributeIsSame(Attribute* attr);

  inline eValueType ValueType() const { return mValueType; }

  bool     operator>>(FILE* fp);
  bool     operator<<(FILE* fp);

private:
  // Data:
  String      mName;          // user will initially retrieve attributes by name
  eValueType  mValueType;     // type of value for this attribute
  short       mBitWidth;      // needed for value type: Bit ? Remove ?
  short       mPosition;      // used as an index into record value array
  Value*      mpAttrNullValue;// support null values (later)
  Table*      mpTable;        // qualify attribute with Table (View?) for Joins etc.
  View*       mpView;         // qualify attribute with a view (when we have multiple views)
};

#endif

// End
