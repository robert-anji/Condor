// $RCSfile: Condition.h,v $
// $Revision: 1.9 $

#ifndef ConditionClass
#define ConditionClass

#include "Attribute.h"
#include "BucketList.h"
#include "Global.h"
#include "Value.h"

class Aggregate;
class Record;
class Value;
class ViewIterator;

enum eCondOperandType {
  eCondOperand_None,
  eCondOperand_Value,
  eCondOperand_ValuePtr,
  eCondOperand_AttributePtr,
  eCondOperand_RecordPtr,
  eCondOperand_Unknown
};

enum eCondOperatorType {
  eCondOper_None,
  eCondOper_Less,
  eCondOper_Greater,
  eCondOper_Equal,
  eCondOper_NotEqual,
  eCondOper_GreaterOrEqual,
  eCondOper_LessOrEqual,
  eCondOper_Like,
  eCondOper_Negation,
  eCondOper_Invalid
};

// A Condition represents one or more comparisons like ValueA > ValueB.
// Note that this class is primed for use with database tables and hence
// one of the vales to compare will come from the database table record,
// the other value can come directly from the users query and is stored
// in this class (mValue) or it can come from another table record (for
// database Join operations.
// If more than one Condition is required then these are gathered inline
// and stored as a sum-of-products using a two-dimensional linked list
// called BucketList.
class Condition
{
  friend class Query;
  friend class ViewJoin;

public:
  Condition(const Attribute* attr);
  Condition(const Aggregate* aggr);
  Condition(const Value  val);
  Condition(const Value* pval);
  Condition(const Condition& c);
  Condition(const Value& val,      bool alloc); // For use by Value class
  Condition(const Attribute* attr, bool alloc); // For use by Attribute class
  Condition(const Aggregate* aggr, bool alloc); // For use by Aggregate class
  //
  ~Condition();

  bool Match(Record* rec=0);

  Condition& operator&&(Condition& c);
  Condition& operator||(Condition& c);

  friend void OperationAnd (BucketList<Condition>& bl1, BucketList<Condition>& bl2);
  friend void OperationOr  (BucketList<Condition>& bl1, BucketList<Condition>& bl2);

  // A Condition object is complete only once a condition operation is invoked
  // on it. This is accomplished for multiple types using the overloaded
  // operators below.
  Condition& operator< (const Value& val);
  Condition& operator> (const Value& val);
  Condition& operator>=(const Value& val);
  Condition& operator<=(const Value& val);
  Condition& operator==(const Value& val);
  Condition& operator!=(const Value& val);
  Condition& operator%=(const Value& val);

  Condition& operator< (const Value* val);
  Condition& operator> (const Value* val);
  Condition& operator>=(const Value* val);
  Condition& operator<=(const Value* val);
  Condition& operator==(const Value* val);
  Condition& operator!=(const Value* val);
  Condition& operator%=(const Value* val);

  Condition& operator< (int val);
  Condition& operator> (int val);
  Condition& operator>=(int val);
  Condition& operator<=(int val);
  Condition& operator==(int val);
  Condition& operator!=(int val);

  Condition& operator< (unsigned val);
  Condition& operator> (unsigned val);
  Condition& operator>=(unsigned val);
  Condition& operator<=(unsigned val);
  Condition& operator==(unsigned val);
  Condition& operator!=(unsigned val);

  Condition& operator< (short val);
  Condition& operator> (short val);
  Condition& operator>=(short val);
  Condition& operator<=(short val);
  Condition& operator==(short val);
  Condition& operator!=(short val);

  Condition& operator< (unsigned short val);
  Condition& operator> (unsigned short val);
  Condition& operator>=(unsigned short val);
  Condition& operator<=(unsigned short val);
  Condition& operator==(unsigned short val);
  Condition& operator!=(unsigned short val);

  Condition& operator< (const Bit& b);
  Condition& operator> (const Bit& b);
  Condition& operator>=(const Bit& b);
  Condition& operator<=(const Bit& b);
  Condition& operator==(const Bit& b);
  Condition& operator!=(const Bit& b);

  Condition& operator< (const char* val);
  Condition& operator> (const char* val);
  Condition& operator>=(const char* val);
  Condition& operator<=(const char* val);
  Condition& operator==(const char* val);
  Condition& operator!=(const char* val);

  Condition& operator< (const Attribute& attr);
  Condition& operator> (const Attribute& attr);
  Condition& operator<=(const Attribute& attr);
  Condition& operator>=(const Attribute& attr);
  Condition& operator==(const Attribute& attr);
  Condition& operator!=(const Attribute& attr);

  Condition& operator==(const Record* val);
  Condition& operator!=(const Record* val);

  Condition& operator==(void* val);
  Condition& operator!=(void* val);

  Condition& operator!();

  // Bit range operator to support bit-select comparison
  Condition& operator[](const char* width_str);

  //
  Condition& EditIndex(short s);

  // Access functions
  eCondOperatorType Operator()   const { return mCondOperator; }
  const Value&     ValueGet()    const { return mValueRHS;     }
  short            GetMsb()      const { return mMsbBit;       }
  short            GetLsb()      const { return mLsbBit;       }
  bool             BitWidthSet() const { return mBitWidthSet;  }
  Value*           ValuePtrGet(const Aggregate* aggr) const;

  // Debug functions
  void Print (FILE* fp);
  void Dump  (FILE* fp);
  void Name  (FILE* fp) { fprintf(fp, "Condition"); }

  inline const Attribute& AttributeGet() { return *mpAttributeLHS; }

private:
  eCondOperandType      mTypeLHS;
  eCondOperandType      mTypeRHS;
  const Attribute*      mpAttributeLHS;    // LHS=>Left-hand-sinde, RHS=>Right-hand-side
  const Attribute*      mpAttributeRHS;
  const Value*          mpValueLHS;
  const Value*          mpValueRHS;
  Value                 mValueLHS;
  Value                 mValueRHS;
//View*                 mpViewLHS;         // needed for joins
//View*                 mpViewRHS;         //         "
  ViewIterator*         mpViewIteratorLHS; //         "
  ViewIterator*         mpViewIteratorRHS; //         "
  eCondOperatorType     mCondOperator;
  BucketList<Condition> mConditionBktList;
  short                 mLsbBit;
  short                 mMsbBit;
  bool                  mBitWidthSet;  
  bool                  mIsAllocatedAuto;
  short                 mValueIndex;

  // Private member functions.

  // Note: Could not make function const because it calls 'rec<mValue' ???
  bool Evaluate();                  // needed for ?
  bool Evaluate(Record* rec);       // needed for typical searches
  bool Evaluate(const Value &lhs, const Value &rhs);


  // Shortcuts for use inside the debugger (gdb):
  void p();
  void pp() {mConditionBktList.p(); }
  void d();
};

#endif

// End
