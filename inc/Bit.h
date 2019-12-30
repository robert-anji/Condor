// $RCSfile: Bit.h,v $
// $Revision: 1.8 $

#ifndef Bit_Class
#define Bit_Class

#include <stdio.h>

#include "DBDefines.h"
#include "Global.h"
#include "SimulatorDefines.h"

class Interface;

class Bit
{
public:
  unsigned mId;
  // Constructors
  Bit();
//Bit(unsigned int i);
  Bit(short width);
  Bit(const Bit& b); // needed for 'passing by value'
  Bit(Interface& intf);
 ~Bit();

  // Determine if the bit satisfies the edge.
  bool IsSensitive(eSensitiveType sens_type, bool is_modified);

  // Assignment
  Bit& operator=(const Bit& b);
  Bit& operator=(Bit& b);
  Bit& operator=(unsigned int value);
  Bit& SelfCopy(); // needed to support b[2] = b[1].SelfCopy()

  // Arithmetic
  Bit& operator++(int);
  Bit& operator--(int);
  void operator+=(Bit &bit);
  void operator-=(Bit &bit);
  Bit& operator+(unsigned u);
  Bit& operator-(unsigned u);
  Bit& operator>>(int u);
  Bit& operator<<(int u);

  // Comparison (int)
  bool operator==(int i);
  bool operator!=(int i);
  bool operator> (int i);
  bool operator< (int i);
  bool operator>=(int i);
  bool operator<=(int i);

  // Comparison (unsigned)
  bool operator==(unsigned u);
  bool operator!=(unsigned u);
  bool operator> (unsigned u);
  bool operator< (unsigned u);
  bool operator>=(unsigned u);
  bool operator<=(unsigned u);

  // Comparison (Bit)
  // Note: cannot have const arg as we clear operand bit (msb,lsb) state
  bool operator==(Bit& bit);
  bool operator!=(Bit& bit);
  bool operator> (Bit& bit);
  bool operator< (Bit& bit);
  bool operator>=(Bit& bit);
  bool operator<=(Bit& bit);

  // Logic operations
  Bit  operator&(Bit& bit);
  Bit  operator|(Bit& bit);
  Bit  operator^(Bit& bit);
  Bit  operator~();
  bool operator!() const;

  // Concatenate
//Bit& Concatenate(Bit &b0, Bit &b1) { Bit b(0,0); /*b=Concatenate2(b0, b1);*/ return b; }
  Bit& Concatenate2(Bit&, Bit&);
  Bit& Concatenate3(Bit&, Bit&, Bit&);
  Bit& Concatenate4(Bit&, Bit&, Bit&, Bit&);
  Bit& Concatenate5(Bit&, Bit&, Bit&, Bit&, Bit&);
  Bit& Concatenate6(Bit&, Bit&, Bit&, Bit&, Bit&, Bit&);
  Bit& Concatenate7(Bit&, Bit&, Bit&, Bit&, Bit&, Bit&, Bit&);
  Bit& Concatenate8(Bit&, Bit&, Bit&, Bit&, Bit&, Bit&, Bit&, Bit&);
  Bit& Concatenate9(Bit&, Bit&, Bit&, Bit&, Bit&, Bit&, Bit&, Bit&, Bit&);

  // Used to set the range of bits for a subsequent operation.
  Bit& operator[](short bit);

  // Set/Test bits
  short IsBitSet      (short bit_num) const;
  void  SetBit        (short bit_num);
  void  ClearBit      (short bit_num);
  void  BitSetAll     ();
  void  BitClearAll   ();

  // Allow retrieving bits as built-in types
  operator unsigned int   ();
  operator unsigned short ();
  operator short          ();
  operator int            ();
  operator bool           ();

  // Return the size of the Bit
  short BitWidth() const  { return mNumBits; }

  // Allow retrieving the commited value from a Signal within an Interface.
  Bit& operator=(Interface& intf);

  // Alternate (faster) way to set msb/lsb instead of using ["n:m"].
  void SetMsbLsb(short msb, short lsb);

  // Save, Initialize, Print the bit.
  char* Save       (char buf [MAX_RECORD_LINE_LENGTH+1]) const;
  bool  Initialize (const char* line, const char* format);
  void  VcdPrint   (FILE* fp) const;
  void  Print      (const char* format_str = "%s\n") const;

protected:
  void        InitMasks();
  void        InitBitsAndWords(short msb, short lsb);

  inline void ResetBitRange()    { mLsbBit=0; mMsbBit=mNumBits-1;
                                   m2ndLsbBit=0; m2ndMsbBit=mMsbBit;
                                   mBitRangeState=eBitRangeState_Clear; }
  inline bool IsEmptyBit() const { if (mNumBits) return FALSE;
                                   else          return TRUE; }

  // Use mutable to allow modification of mLsbBit & mMsbBit in const functions.
  mutable short      mLsbBit;
  mutable short      mMsbBit;
  mutable short      m2ndLsbBit; // needed for operations on self
  mutable short      m2ndMsbBit; //               "

  short      mNumWords;
  unsigned*  mpWords;
  short      mNumBits; // Note:
                       // mNumBits is needed because mMsbBit and mLsbBit can
                       // be changed when working with bit ranges.

  Bit*       mpSavedBit;
  bool       mSavedBitSet;

  enum eBitRangeState {
    eBitRangeState_Clear,
    eBitRangeState_MsbSet,
    eBitRangeState_LsbSet,
    eBitRangeState_2ndMsbSet,
    eBitRangeState_2ndLsbSet,
    eBitRangeState_Invalid
  };

  eBitRangeState mBitRangeState;
};

class Bit1  : public Bit { public: Bit1()  : Bit( 0) {} ~Bit1()  {} };
class Bit2  : public Bit { public: Bit2()  : Bit( 1) {} ~Bit2()  {} };
class Bit3  : public Bit { public: Bit3()  : Bit( 2) {} ~Bit3()  {} };
class Bit4  : public Bit { public: Bit4()  : Bit( 3) {} ~Bit4()  {} };
class Bit5  : public Bit { public: Bit5()  : Bit( 4) {} ~Bit5()  {} };
class Bit6  : public Bit { public: Bit6()  : Bit( 5) {} ~Bit6()  {} };
class Bit7  : public Bit { public: Bit7()  : Bit( 6) {} ~Bit7()  {} };
class Bit8  : public Bit { public: Bit8()  : Bit( 7) {} ~Bit8()  {} };
class Bit9  : public Bit { public: Bit9()  : Bit( 8) {} ~Bit9()  {} };
class Bit10 : public Bit { public: Bit10() : Bit( 9) {} ~Bit10() {} };
class Bit11 : public Bit { public: Bit11() : Bit(10) {} ~Bit11() {} };
class Bit12 : public Bit { public: Bit12() : Bit(11) {} ~Bit12() {} };
class Bit13 : public Bit { public: Bit13() : Bit(12) {} ~Bit13() {} };
class Bit14 : public Bit { public: Bit14() : Bit(13) {} ~Bit14() {} };
class Bit15 : public Bit { public: Bit15() : Bit(14) {} ~Bit15() {} };
class Bit16 : public Bit { public: Bit16() : Bit(15) {} ~Bit16() {} };
class Bit17 : public Bit { public: Bit17() : Bit(16) {} ~Bit17() {} };
class Bit18 : public Bit { public: Bit18() : Bit(17) {} ~Bit18() {} };
class Bit19 : public Bit { public: Bit19() : Bit(18) {} ~Bit19() {} };
class Bit20 : public Bit { public: Bit20() : Bit(19) {} ~Bit20() {} };
class Bit21 : public Bit { public: Bit21() : Bit(20) {} ~Bit21() {} };
class Bit22 : public Bit { public: Bit22() : Bit(21) {} ~Bit22() {} };
class Bit23 : public Bit { public: Bit23() : Bit(22) {} ~Bit23() {} };
class Bit24 : public Bit { public: Bit24() : Bit(23) {} ~Bit24() {} };
class Bit25 : public Bit { public: Bit25() : Bit(24) {} ~Bit25() {} };
class Bit26 : public Bit { public: Bit26() : Bit(25) {} ~Bit26() {} };
class Bit27 : public Bit { public: Bit27() : Bit(26) {} ~Bit27() {} };
class Bit28 : public Bit { public: Bit28() : Bit(27) {} ~Bit28() {} };
class Bit29 : public Bit { public: Bit29() : Bit(28) {} ~Bit29() {} };
class Bit30 : public Bit { public: Bit30() : Bit(29) {} ~Bit30() {} };
class Bit31 : public Bit { public: Bit31() : Bit(30) {} ~Bit31() {} };
class Bit32 : public Bit { public: Bit32() : Bit(31) {} ~Bit32() {} };
class Bit64 : public Bit { public: Bit64() : Bit(63) {} ~Bit64() {} };

#endif
// End
