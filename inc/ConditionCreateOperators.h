// $RCSfile: ConditionCreateOperators.h,v $
// $Revision: 1.1 $

// If a class wishes to be built into a Query it needs to be able to create
// Condition objects. The operators below are the declarations needed.

Condition& operator< (unsigned val) const;
Condition& operator> (unsigned val) const;
Condition& operator>=(unsigned val) const;
Condition& operator<=(unsigned val) const;
Condition& operator==(unsigned val) const;
Condition& operator!=(unsigned val) const;

Condition& operator< (int val) const;
Condition& operator> (int val) const;
Condition& operator>=(int val) const;
Condition& operator<=(int val) const;
Condition& operator==(int val) const;
Condition& operator!=(int val) const;

Condition& operator< (short val) const;
Condition& operator> (short val) const;
Condition& operator>=(short val) const;
Condition& operator<=(short val) const;
Condition& operator==(short val) const;
Condition& operator!=(short val) const;

Condition& operator< (char* val) const;
Condition& operator> (char* val) const;
Condition& operator>=(char* val) const;
Condition& operator<=(char* val) const;
Condition& operator==(char* val) const;
Condition& operator!=(char* val) const;

Condition& operator< (const char* val) const;
Condition& operator> (const char* val) const;
Condition& operator>=(const char* val) const;
Condition& operator<=(const char* val) const;
Condition& operator==(const char* val) const;
Condition& operator!=(const char* val) const;

Condition& operator==(void* val) const;
Condition& operator!=(void* val) const;

Condition& operator!() const;

// End
