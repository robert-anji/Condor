// $RCSfile: Bit.cc,v $
// $Revision: 1.6 $

#include <assert.h>
#include <stdio.h>
#include <bits/wordsize.h>
#include <limits.h>
#include <string.h>

#include "Bit.h"
//
#include "Global.h"
#include "Interface.h"
#include "SimulatorDefines.h"

const short cWordSize              = __WORDSIZE;
const short cWordSizeMinus1        = __WORDSIZE-1;
const short cCharsPerWordHex       = cWordSize/4; // since 4 hex bits per char
const short cWordSizeLog           = 5; // couldnt find #define, verify in Bit()
const unsigned cWordMax            = UINT_MAX;

// The following cannot be declared 'const' because we initialize them later.
static unsigned int cBitSetAt[32];    // This sets up bit patterns like:
                                      // [0] 00000000_00000000_00000000_00000001
                                      // [1] 00000000_00000000_00000000_00000010
                                      // [2] 00000000_00000000_00000000_00000100
                                      // etc.
static unsigned int cBitNotSetAt[32]; // This sets up bit patterns like:
                                      // [0] 11111111_11111111_11111111_11111110
                                      // [1] 11111111_11111111_11111111_11111101
                                      // [2] 11111111_11111111_11111111_11111011
                                      // etc.
static unsigned int cBitSetUpto[32];  // This sets up bit patterns like:
                                      // [0] 00000000_00000000_00000000_00000001
                                      // [1] 00000000_00000000_00000000_00000011
                                      // [2] 00000000_00000000_00000000_00000111
                                      // etc.
static short cBitSetAtInitDone = FALSE;

M_ExternConstructDestructCountInt;

extern Bit* gSavedBit;
static int gConstructCountBit;

Bit::Bit() :
  mNumWords  (0),
  mNumBits   (0),
  mLsbBit    (0),
  mMsbBit    (0),
  mpWords    (0),
  mSavedBitSet(FALSE),
  mBitRangeState(eBitRangeState_Clear)
{
  if (!cBitSetAtInitDone) InitMasks();

  // This (default) constructor creates an empty bit that can be sized later
  //       The reason for this constructor was simplified Concatenate logic,
  //       we will not need to calculate the Bit width before assigning the
  //       concatenated bits.
  //       In general, whenever we dont yet know the size of the bit vector we
  //       can use this instance.
  //       Also need for Bit arrays as C++ arrays always use constructor X::X().

  mId = gConstructCountBit++;

  M_UpdConstructCount(eType_Bit);
}

Bit::Bit(short width) :
  mNumWords  (0),
  mNumBits   (0),
  mLsbBit    (0),
  mMsbBit    (0),
  mpWords    (0),
  mSavedBitSet(FALSE),
  mBitRangeState(eBitRangeState_Clear)
{
  mId = gConstructCountBit++;
  assert(width);

  // We need a one-time global initialization for this class.
  if (!cBitSetAtInitDone) InitMasks();

  short low_bit;

  InitBitsAndWords(width-1, low_bit=0);

  assert(mpWords);

  M_UpdConstructCount(eType_Bit);
}

Bit::Bit(const Bit &b) :
  mNumWords  (0),
  mNumBits   (0),
  mLsbBit    (0),
  mMsbBit    (0),
  mpWords    (0),
  mSavedBitSet(FALSE),
  mBitRangeState(eBitRangeState_Clear)
{
  mId = gConstructCountBit++;
  // Nothing more to do if 'b' is empty.
  if (b.IsEmptyBit()) return;

  // Size this object based on incoming msb, lsb bits, not mNumBits.
  InitBitsAndWords(b.mMsbBit, b.mLsbBit);

  for (short i=0; i<mNumWords; i++)
    mpWords[i] = b.mpWords[i];

  M_UpdConstructCount(eType_Bit);
}

Bit::Bit(Interface& intf) :
  mNumWords  (0),
  mNumBits   (0),
  mLsbBit    (0),
  mMsbBit    (0),
  mpWords    (0),
  mSavedBitSet(FALSE),
  mBitRangeState(eBitRangeState_Clear)
{
  mId = gConstructCountBit++;
  // The bit will be sized within GetCommittedValue() using operator=(Bit&).
  intf.GetCommittedValue(*this);

  M_UpdConstructCount(eType_Bit);
}

Bit::~Bit()
{
  delete [] mpWords; mpWords = 0; M_UpdDestructCountN(eType_Unsigned, mNumWords);

  mNumWords = 0;
  mNumBits  = 0;
  mMsbBit   = 0;
  mLsbBit   = 0;

  printf("BIT: Destructed mId=%u\n", mId);
  M_UpdDestructCount(eType_Bit);
}

// Causes problems with bit = (Bit)value; // in Interface.cc
//Bit::Bit(unsigned int i) :
//  mNumWords  (1),
//  mNumBits   (32),
//  mLsbBit    (0),
//  mMsbBit    (31),
//  mpWords    (0),
//  mBitRangeState(eBitRangeState_Clear)
//{
//  // Size this object based on incoming msb, lsb bits, not mNumBits.
//  InitBitsAndWords(mMsbBit, mLsbBit);
//
//  mpWords[0] = i;
//}

void Bit::InitBitsAndWords(short msb, short lsb)
{
  if (lsb > msb) { short tmp = lsb; lsb = msb; msb = tmp; }

  mNumBits  = 1 + msb - lsb;
  mNumWords = 1 + ((mNumBits-1)>>cWordSizeLog);
  mpWords   = new unsigned [mNumWords]; M_UpdConstructCountN(eType_Unsigned, mNumWords);

  for (short i=0; i<mNumWords; i++) mpWords[i] = 0;

  mLsbBit   = 0;
  mMsbBit   = mNumBits - 1;

  assert(mpWords);
}

void Bit::InitMasks()
{
  unsigned int mask = 1;

  for (int i=0; i<cWordSize; i++) {
    cBitSetAt[i]    =  mask;
    cBitNotSetAt[i] = ~mask;

    if (i == 0) cBitSetUpto[i] = 1;
    else        cBitSetUpto[i] = (cBitSetUpto[i-1]<<1) | 1;

    mask = mask << 1;
  }

  cBitSetAtInitDone = TRUE;

  // Confirm consistent constants.
  assert((1<<cWordSizeLog) == cWordSize);
}

bool Bit::Initialize(const char* line, const char* format)
{
  // TBD: The Initialize task will not work well if the bit is not yet sized.

  short chars_total    = strlen(line);
  
  unsigned      u;
  bool          status;
  const char   *char_ptr;
  short         word_count;
  short         char_count;
  short         num_bits;

  if (IsEmptyBit()) {
    if      (*format=='x') num_bits = chars_total*4;
    else if (*format=='d') num_bits = chars_total*4; // TBD: need less bits here

    InitBitsAndWords(num_bits-1, 0);
  }

  for (u=0, status=TRUE, char_ptr=line+chars_total, word_count=0, char_count=0;
   status && char_ptr>=line && word_count<mNumWords;
    char_ptr--, char_count++)
  {
    if (MacroIsExactMultiple(char_count, cCharsPerWordHex)) {
      mpWords[word_count++] = u;
      u                     = 0;
    }
    else {
      u = u << 4; // make room for next 4 bits since 4 bits per hex char
    }

    char c = *char_ptr;

    if      (c>='0' && c<='9') u = u | (c - '0');
    else if (c>='a' && c<='f') u = u | (c - 'a');
    else if (c>='A' && c<='F') u = u | (c - 'A');
    else                       status = FALSE;
  }
  // TBD: confirm 'line' len matches bit_width

  if (char_ptr >= line) status = FALSE; // not all char's consumed

  return status;
}

bool Bit::IsSensitive(eSensitiveType sens_type, bool is_modified)
{
  if (sens_type == eSensitiveType_ValueChange) {
    return (is_modified);
  }
  else {
    // The remaining sensitive types are all for single bit.
    assert (mMsbBit == 0);

    switch (sens_type) {
      case eSensitiveType_PositiveEdge:
      case eSensitiveType_PositiveLevel:           return *this == 1; break;
      case eSensitiveType_NegativeEdge:
      case eSensitiveType_NegativeLevel:           return *this == 0; break;
      case eSensitiveType_PositiveAndNegativeEdge: return TRUE;
      default:                                     assert(0);
    }
  }

  return FALSE;
}

// Note: This logic is also copied to operator=(Bit& b).
Bit& Bit::operator=(Bit& b)
{
  // If assigning to self then handle it specially
  // as we may be assigning different sets of bits.
  if (&b == this) { 
    // When assigning to self ensure mSavedBitSet was set using SelfCopy().
    if (mSavedBitSet) { mSavedBitSet = FALSE; }
    else {
      short msb1, lsb1, msb2, lsb2;
    //*gSavedBit[m2ndMsbBit][m2ndLsbBit] = *this[m2ndMsbBit][m2ndLsbBit]; // problem: infinite recursion // FIX
      mSavedBitSet = TRUE;
    //return *this[msb2][lsb2] = *gSavedBit[msb1][lsb1]; // FIX
    //return *this[msb2][lsb2] = 0; // FIX
    }
  // CONT:
  // gSavedBit = *this[mMsbBit][mLsbBit]; // problem: infinite recursion
  // short m=m2ndMsbBit, l=m2ndLsbBit;
  // ResetBitRange();
  // *this[m][l] = gSavedBit;
  }

  // Initially thought it doesnt make sense to assign an empty bit
  // but we allow now because:
  // - Bit can be sized at runtime and copy may be made before then
  // - During development a class's Bit may become unused (not removed)
  if (b.IsEmptyBit()) {
    delete [] mpWords; M_UpdDestructCountN(eType_Unsigned, mNumWords);

    mNumWords = mNumBits = mLsbBit = mMsbBit = m2ndLsbBit = m2ndMsbBit = 0;

    return *this;
  }

  // If this Bit is not yet sized, size it now (same size as 'b').
  if (mpWords==0) InitBitsAndWords(b.mMsbBit, b.mLsbBit);

  // If both bits are equal size and we are copying the
  // full range of bits then simply copy over the words.
  if (mMsbBit==b.mMsbBit && mLsbBit==b.mLsbBit && mNumBits==(mMsbBit-mLsbBit+1)) {
    for (int i=0; i<mNumWords; i++) {
      mpWords[i] = b.mpWords[i];
    }

    return *this;
  }

  if ((b.mMsbBit-b.mLsbBit) > (mMsbBit-mLsbBit))
  {
    // We dont allow assignment if bit width of operand is greater.
    assert(0);
  }
  else {
    short i =   mLsbBit;
    short m =   mLsbBit>>cWordSizeLog; // mpWords index
    short j = b.mLsbBit;
    short n = b.mLsbBit>>cWordSizeLog; // mpWords index

    for (; i<=mMsbBit; i++,j++)
    {
      if (j > b.mMsbBit) {
        mpWords[m] = mpWords[m] & cBitNotSetAt[i%cWordSize];
      }
      else {
        if (b.mpWords[n] & cBitSetAt[j%cWordSize])
          mpWords[m] = mpWords[m] | cBitSetAt[i%cWordSize];
        else
          mpWords[m] = mpWords[m] & cBitNotSetAt[i%cWordSize];
      }

      if ((i%cWordSize) == (cWordSize-1)) m++;
      if ((j%cWordSize) == (cWordSize-1)) n++;
    }
  }

  ResetBitRange();
  b.ResetBitRange();

  return *this;
}

// This assignment operator is invoked whenever 'copy by value' is needed.
// Note: logic is copied from operator=(Bit& b).
Bit& Bit::operator=(const Bit& b)
{
  // If assigning to self then there is not much to do.
  if (&b == this) { ResetBitRange(); return *this; }

  // I think it doesnt make sense to assign an empty bit
  assert(! b.IsEmptyBit());

  // If this Bit is not yet sized, size it now.
  if (mpWords == 0) InitBitsAndWords(b.mMsbBit, b.mLsbBit);

  // If both bits are equal size and we are copying the
  // full range of bits then simply copy over the words.
  if (mMsbBit==b.mMsbBit && mLsbBit==b.mLsbBit && mNumBits==(mMsbBit-mLsbBit+1)) {
    for (int i=0; i<mNumWords; i++) {
      mpWords[i] = b.mpWords[i];
    }

    return *this;
  }

  if ((mMsbBit-mLsbBit) < (b.mMsbBit-b.mLsbBit))
  {
    // We dont allow assignment if bit width of operand is greater
    assert(0);
  }
  else {
    short i =   mLsbBit;
    short m =   mLsbBit>>cWordSizeLog; // mpWords index
    short j = b.mLsbBit;
    short n = b.mLsbBit>>cWordSizeLog; // mpWords index

    for (; i<=mMsbBit; i++,j++)
    {
      if (b.mpWords[n] & cBitSetAt[j%cWordSize])
        mpWords[m] = mpWords[m] | cBitSetAt[i%cWordSize];
      else
        mpWords[m] = mpWords[m] & cBitNotSetAt[i%cWordSize];

      if ((i%cWordSize) == (cWordSize-1)) m++;
      if ((j%cWordSize) == (cWordSize-1)) n++;
    }
  }

  ResetBitRange();

  return *this;
}

Bit& Bit::operator=(unsigned u)
{
  // If this Bit is not yet sized, size it now.
  if (mpWords == 0) InitBitsAndWords(cWordSizeMinus1, 0);

  short w0 = mLsbBit >> ((cWordSize==64) ? 6 : 5);
  short w1 = mMsbBit >> ((cWordSize==64) ? 6 : 5);

  short width_minus1 = mMsbBit - mLsbBit;

  // We dont allow truncation so confirm that u fits in 'width' bits,
  // (use width-1 because shifting by 32 will not work as expected with gcc).
  assert((u >> width_minus1) <= 1);

  if (w0 == w1) {
    // msb and lsb are in the same word.
    mpWords[w0] = mpWords[w0] & ~(cBitSetUpto[width_minus1]<<(mLsbBit%32))  | u<<mLsbBit%32;
  }
  else {
    // msb and lsb are not in the same word, first assign lower word.
    short width_w0 = cWordSize - (mLsbBit%32);
    mpWords[w0] = mpWords[w0] & ~(cBitSetUpto[width_w0]<<(mLsbBit%32)) | u<<mLsbBit%32;

    // now assign upper word.
    short width_w1 = mMsbBit%32;
    mpWords[w1] = mpWords[w1] & ~cBitSetUpto[width_w0] | u>>width_w0;
  }

  ResetBitRange();

  return *this;
}

// CONT: Fix comparison operators not accounting for msb, lsb.
bool Bit::operator==(int ii)
{
  assert(ii >= 0); // we dont support -ve values inside Bit

  // First, special handling if Bit is still empty (never sized or assigned).
  if (IsEmptyBit()) return FALSE;

  bool flag;

  short msb_word_num = mMsbBit>>cWordSizeLog;
  short lsb_word_num = mLsbBit>>cWordSizeLog;
  //
  short bit_range_straddles = mNumWords>1 || (msb_word_num!=lsb_word_num);

  if (!bit_range_straddles) {
    if (mpWords[msb_word_num] == ii) flag = TRUE;
    else                             flag = FALSE;
  }
  else assert(0); // not yet implemented
  
  ResetBitRange();

  return flag;
}

bool Bit::operator!=(int ii)
{
  return ! (*this == ii);
}

bool Bit::operator>(int ii)
{
  assert(ii >= 0); // we dont support -ve values inside Bit

  // First, special handling if Bit is still empty (never sized or assigned).
  if (IsEmptyBit()) return FALSE;

  bool flag;

  short msb_word_num = mMsbBit>>cWordSizeLog;
  short lsb_word_num = mLsbBit>>cWordSizeLog;
  //
  short bit_range_straddles = mNumWords>1 || (msb_word_num!=lsb_word_num);

  if (!bit_range_straddles) {
    if ((mpWords[msb_word_num]&cBitSetUpto[mMsbBit])>>mLsbBit > ii) flag = TRUE;
    else flag = FALSE;
  }
  else assert(0); // not yet implemented
  
  ResetBitRange();

  return flag;
}

bool Bit::operator<(int ii)
{
  assert(ii >= 0); // we dont support -ve values inside Bit

  // First, special handling if Bit is still empty (never sized or assigned).
  if (IsEmptyBit()) return FALSE;

  bool flag;

  short msb_word_num = mMsbBit>>cWordSizeLog;
  short lsb_word_num = mLsbBit>>cWordSizeLog;
  //
  short bit_range_straddles = mNumWords>1 || (msb_word_num!=lsb_word_num);

  if (!bit_range_straddles) {
    if (mpWords[msb_word_num] < ii) flag = TRUE;
    else                            flag = FALSE;
  }
  else assert(0); // not yet implemented
  
  ResetBitRange();

  return flag;
}

bool Bit::operator>=(int ii)
{
  return ! (*this < ii);
}

bool Bit::operator<=(int ii)
{
  return ! (*this > ii);
}

bool Bit::operator==(unsigned u)
{
  // First, special handling if Bit is still empty (never sized or assigned).
  if (IsEmptyBit()) return FALSE;

  bool flag;

  short msb_word_num = mMsbBit>>cWordSizeLog;
  short lsb_word_num = mLsbBit>>cWordSizeLog;
  //
  short bit_range_straddles = mNumWords>1 || (msb_word_num!=lsb_word_num);

  if (!bit_range_straddles) {
    if (mpWords[msb_word_num] == u) flag = TRUE;
    else                            flag = FALSE;
  }
  else assert(0); // not yet implemented
  
  ResetBitRange();

  return flag;
}

bool Bit::operator!=(unsigned u)
{
  return ! (*this == u);
}

bool Bit::operator>(unsigned u)
{
  // First, special handling if Bit is still empty (never sized or assigned).
  if (IsEmptyBit()) return FALSE;

  bool flag;

  short msb_word_num = mMsbBit>>cWordSizeLog;
  short lsb_word_num = mLsbBit>>cWordSizeLog;
  //
  short bit_range_straddles = mNumWords>1 || (msb_word_num!=lsb_word_num);

  if (!bit_range_straddles) {
    if (mpWords[msb_word_num] > u) flag = TRUE;
    else                           flag = FALSE;
  }
  else assert(0); // not yet implemented
  
  ResetBitRange();

  return flag;
}

bool Bit::operator<(unsigned u)
{
  // First, special handling if Bit is still empty (never sized or assigned).
  if (IsEmptyBit()) return FALSE;

  bool flag;

  short msb_word_num = mMsbBit>>cWordSizeLog;
  short lsb_word_num = mLsbBit>>cWordSizeLog;
  //
  short bit_range_straddles = mNumWords>1 || (msb_word_num!=lsb_word_num);

  if (!bit_range_straddles) {
    if (mpWords[msb_word_num] < u) flag = TRUE;
    else                           flag = FALSE;
  }
  else assert(0); // not yet implemented
  
  ResetBitRange();

  return flag;
}

bool Bit::operator>=(unsigned u)
{
  return ! (*this < u);
}

bool Bit::operator<=(unsigned u)
{
  return ! (*this > u);
}

bool Bit::operator==(Bit& b)
{
  // Note: this code is reused for the other comparison operators,
  //       so changes here should get reflected there.

  short    num_words;
  short    i;
  bool     flag;

  // First, special handling if either Bit is empty.
  // We allow equality check for empty bits but not for comparison checks.
  if (IsEmptyBit() || b.IsEmptyBit()) {
    if (IsEmptyBit() && b.IsEmptyBit()) return TRUE;
    else                                return FALSE;
  }
  
  // Optimization is possible when Bit's have same msb & lsb, and lsb=0.
  if (mMsbBit==b.mMsbBit && mLsbBit==b.mLsbBit && mLsbBit==0) {
    num_words = mMsbBit>>cWordSizeLog;
    flag      = TRUE;

    for (i=0; i<=num_words; i++) {
      if (mpWords[i] != b.mpWords[i]) { flag = FALSE; break; }
    }

    ResetBitRange(); b.ResetBitRange();
    return flag;
  }

  // We should compare the value only, the
  // number of bits in each can be different.
  short width1 =   mMsbBit -   mLsbBit; // not the actual width but one less
  short width2 = b.mMsbBit - b.mLsbBit; //                "

  short j, m, n;

  for (i=mMsbBit;   width1 > width2; i--, width1--) {
    m = i>>cWordSizeLog;
    if (mpWords[m] & cBitSetAt[i%cWordSize]) {
      ResetBitRange(); b.ResetBitRange(); return FALSE;
    }
  }

  for (j=b.mMsbBit; width2 > width1; j--, width2--) {
    m = j>>cWordSizeLog;
    if (b.mpWords[m] & cBitSetAt[j%cWordSize]) {
      ResetBitRange(); b.ResetBitRange(); return FALSE;
    }
  }

  // If we reach here, then width1 == width2
  for ( ; TRUE; i--, j--) {
    m = i>>cWordSizeLog;
    n = j>>cWordSizeLog;
    if (( (mpWords[m] & cBitSetAt[i%cWordSize]) && !(b.mpWords[n] & cBitSetAt[j%cWordSize]))
    ||  (!(mpWords[m] & cBitSetAt[i%cWordSize]) &&  (b.mpWords[n] & cBitSetAt[j%cWordSize])))
    {
      ResetBitRange(); b.ResetBitRange(); return FALSE;
    }

    // Can test for either 'i' or 'j' .
    if (i == mLsbBit) break;
  }

  ResetBitRange(); b.ResetBitRange(); return TRUE; // done comparing all bits
}

bool Bit::operator!=(Bit& b)
{
  return !(*this == b);
}

bool Bit::operator>(Bit& b)
{
  // Note: code copied from operator==

  short    num_words;
  short    i;
  bool     flag;
  
  // First, special handling if either Bit is empty.
  // We allow equality check for empty bits but not for comparison checks.
  assert (!IsEmptyBit() && !b.IsEmptyBit()); // don't allow comparison if empty

  // Optimization is possible when Bit's have same msb & lsb, and lsb=0.
  if (mMsbBit==b.mMsbBit && mLsbBit==b.mLsbBit && mLsbBit==0) {
    num_words = mMsbBit>>cWordSizeLog;
    flag      = FALSE;

    for (i=num_words; i>=0; i--) {
      if (mpWords[i] > b.mpWords[i]) { flag = TRUE; break; }
    }

    ResetBitRange(); b.ResetBitRange();
    return flag;
  }

  // We should compare the value only, the
  // number of bits in each can be different.
  short width1 =   mMsbBit -   mLsbBit; // not the actual width but width-1
  short width2 = b.mMsbBit - b.mLsbBit; //                "

  short j, m, n;

  for (i=mMsbBit;   width1 > width2; i--, width1--) {
    m = i>>cWordSizeLog;
    if (mpWords[m] & cBitSetAt[i%cWordSize]) {
      ResetBitRange(); b.ResetBitRange(); return TRUE;
    }
  }

  for (j=b.mMsbBit; width2 > width1; j--, width2--) {
    m = j>>cWordSizeLog;
    if (b.mpWords[m] & cBitSetAt[j%cWordSize]) {
      ResetBitRange(); b.ResetBitRange(); return FALSE;
    }
  }

  // If we reach here, then width1 == width2
  for ( ; TRUE; i--, j--) {
    m = i>>cWordSizeLog;
    n = j>>cWordSizeLog;
    if ((  mpWords[m] & cBitSetAt[i%cWordSize])
    && !(b.mpWords[n] & cBitSetAt[j%cWordSize]))
    {
      ResetBitRange(); b.ResetBitRange(); return TRUE;
    }

    // Can test for either 'i' or 'j' .
    if (i==mLsbBit) break;
  }

  ResetBitRange(); b.ResetBitRange(); return FALSE; // done comparing all bits, bits are <=
}

bool Bit::operator<(Bit& b)
{
  // Note: code copied from operator==

  // First, special handling if either Bit is empty.
  // We allow equality check for empty bits but not for comparison checks.
  assert (!IsEmptyBit() && !b.IsEmptyBit()); // don't allow comparison if empty

  // Optimization is possible when Bit's have same msb & lsb, and lsb=0.
  short    num_words;
  short    i;
  bool     flag;
  
  if (mMsbBit==b.mMsbBit && mLsbBit==b.mLsbBit && mLsbBit==0) {
    num_words = mMsbBit>>cWordSizeLog;
    flag      = FALSE;

    for (i=num_words; i>=0; i--) {
      if (mpWords[i] < b.mpWords[i]) { flag = TRUE; break; }
    }

    ResetBitRange(); b.ResetBitRange();
    return flag;
  }

  // We should compare the value only, the
  // number of bits in each can be different.
  short width1 =   mMsbBit -   mLsbBit; // not the actual width but width-1
  short width2 = b.mMsbBit - b.mLsbBit; //                "

  short j, m, n;

  for (i=mMsbBit;   width1 > width2; i--, width1--) {
    m = i>>cWordSizeLog;
    if (mpWords[m] & cBitSetAt[i%cWordSize]) {
      ResetBitRange(); b.ResetBitRange(); return FALSE;
    }
  }

  for (j=b.mMsbBit; width2 > width1; j--, width2--) {
    m = j>>cWordSizeLog;
    if (b.mpWords[m] & cBitSetAt[j%cWordSize]) {
      ResetBitRange(); b.ResetBitRange(); return TRUE;
    }
  }

  // If we reach here, then width1 == width2
  for ( ; TRUE; i--, j--) {
    m = i>>cWordSizeLog;
    n = j>>cWordSizeLog;
    if (!(  mpWords[m] & cBitSetAt[i%cWordSize])
    &&   (b.mpWords[n] & cBitSetAt[j%cWordSize]))
    {
      ResetBitRange(); b.ResetBitRange(); return TRUE;
    }

    // Can test for either 'i' or 'j' .
    if (i==mLsbBit) break;
  }

  ResetBitRange(); b.ResetBitRange(); return FALSE; // done comparing all bits, bits are >=
}

bool Bit::operator>=(Bit& b)
{
  return ! (*this < b);
}

bool Bit::operator<=(Bit& b)
{
  return ! (*this > b);
}

Bit& Bit::Concatenate2(Bit &b1, Bit &b0)
{
  bool empty0 = b0.IsEmptyBit();
  bool empty1 = b1.IsEmptyBit();

  short concat_bit_width;

  // Take care of empty bits.
  concat_bit_width  = (empty0!=0) + b0.mMsbBit-b0.mLsbBit;
  concat_bit_width += (empty1!=0) + b1.mMsbBit-b1.mLsbBit;

  if (concat_bit_width == 0) {
    if (IsEmptyBit()) return *this; // leave as empty
    else              assert(0);    // cannot concatenate a non-empty to empty
  }

  // If this Bit is not yet sized, size it now.
  if (mpWords == 0) InitBitsAndWords(concat_bit_width-1, 0);

  // Now make sure this bit is same size as needed by concatenation.
  assert(mMsbBit-mLsbBit+1 == concat_bit_width);

  (*this)[mLsbBit+concat_bit_width-1][mLsbBit] = b0;
  (*this)[mMsbBit][mLsbBit+concat_bit_width]   = b1;

  return *this;
}

Bit& Bit::Concatenate3(Bit &b2, Bit &b1, Bit &b0)
{
  Bit b; M_Constructed(Bit);

  Concatenate2(b2, b.Concatenate2(b1, b0));

  return *this;
}

Bit& Bit::Concatenate4(Bit &b3, Bit &b2, Bit &b1, Bit &b0)
{
  Bit b3_2;
  Bit b1_0;

  Concatenate2(b3_2.Concatenate2(b3, b2), b1_0.Concatenate2(b1, b0));

  return *this;
}

Bit& Bit::Concatenate5(Bit &b4, Bit &b3, Bit &b2, Bit &b1, Bit &b0)
{
  Bit b3_2;
  Bit b1_0;

  Concatenate3(b4, b3_2.Concatenate2(b3, b2), b1_0.Concatenate2(b1, b0));

  return *this;
}

Bit& Bit::Concatenate6(Bit &b5, Bit &b4, Bit &b3, Bit &b2, Bit &b1, Bit &b0)
{
  Bit b5_4;
  Bit b3_2;
  Bit b1_0;

  b5_4.Concatenate2(b5, b4);
  b3_2.Concatenate2(b3, b2);
  b1_0.Concatenate2(b1, b0);

  Bit b3_0;

  Concatenate2(b5_4, (b3_0.Concatenate2(b3_2, b1_0)));

  return *this;
}

Bit Bit::operator&(Bit& bit)
{
  assert(!IsEmptyBit());
  assert((mMsbBit-mLsbBit) == (bit.mMsbBit-bit.mLsbBit));

  Bit b(*this); M_Constructed(Bit);

  short i = mLsbBit;
  short j = bit.mLsbBit;
  short m;
  short n;

  // OPT: directly 'and' the mpWords instead of individual bits.
  for (; i<=b.mMsbBit; i++, j++) {
    m = i>>cWordSizeLog;
    n = j>>cWordSizeLog;

    if ((  b.mpWords[m] & cBitSetAt[i%cWordSize])
    &&  (bit.mpWords[n] & cBitSetAt[j%cWordSize]))
    {
      b.mpWords[m] = b.mpWords[m] | cBitSetAt[i%cWordSizeLog];
    }
    else {
      b.mpWords[m] = b.mpWords[m] & cBitNotSetAt[i%cWordSizeLog];
    }
  }

  ResetBitRange();
  bit.ResetBitRange();

  return b;
}

Bit Bit::operator|(Bit& bit)
{
  assert(!IsEmptyBit());
  assert((mMsbBit-mLsbBit) == (bit.mMsbBit-bit.mLsbBit));

  Bit b(*this); M_Constructed(Bit);

  short i = mLsbBit;
  short j = bit.mLsbBit;
  short m;
  short n;

  // OPT: directly 'or' the mpWords instead of individual bits.
  for (; i<=b.mMsbBit; i++, j++) {
    m = i>>cWordSizeLog;
    n = j>>cWordSizeLog;

    if ((  b.mpWords[m] & cBitSetAt[i%cWordSize])
    ||  (bit.mpWords[n] & cBitSetAt[j%cWordSize]))
    {
      b.mpWords[m] = b.mpWords[m] | cBitSetAt[i%cWordSizeLog];
    }
    else {
      b.mpWords[m] = b.mpWords[m] & cBitNotSetAt[i%cWordSizeLog];
    }
  }

  ResetBitRange();
  bit.ResetBitRange();

  return b;
}

Bit Bit::operator^(Bit& bit)
{
  assert(!IsEmptyBit());
  assert((mMsbBit-mLsbBit) == (bit.mMsbBit-bit.mLsbBit));

  Bit b(*this); M_Constructed(Bit);

  short i = mLsbBit;
  short j = bit.mLsbBit;
  short m;
  short n;

  // OPT: directly 'or' the mpWords instead of individual bits.
  for (; i<=b.mMsbBit; i++, j++) {
    m = i>>cWordSizeLog;
    n = j>>cWordSizeLog;

    if ((   (  b.mpWords[m] & cBitSetAt[i%cWordSize])
         && (bit.mpWords[n] & cBitNotSetAt[j%cWordSize]))
    ||  (   (  b.mpWords[m] & cBitNotSetAt[i%cWordSize])
         && (bit.mpWords[n] & cBitSetAt[j%cWordSize])))
    {
      b.mpWords[m] = b.mpWords[m] | cBitSetAt[i%cWordSizeLog];
    }
    else {
      b.mpWords[m] = b.mpWords[m] & cBitNotSetAt[i%cWordSizeLog];
    }
  }

  ResetBitRange();
  bit.ResetBitRange();

  return b;
}

Bit Bit::operator~()
{
  Bit b(*this); M_Constructed(Bit);

  short i = mLsbBit;
  short m;

  // OPT: directly 'or' the mpWords instead of individual bits.
  for (; i<=b.mMsbBit; i++) {
    m = i>>cWordSizeLog;

    if (b.mpWords[m] & cBitSetAt[i%cWordSize])
      b.mpWords[m] = b.mpWords[m] & cBitNotSetAt[i%cWordSizeLog];
    else
      b.mpWords[m] = b.mpWords[m] | cBitSetAt[i%cWordSizeLog];
  }

  ResetBitRange();

  return b;
}

bool Bit::operator!() const
{
  for (short i=0; i<mNumWords; i++)
    if (mpWords[i] != 0) return FALSE;

  return TRUE;
}

Bit& Bit::operator++(int)
{
  assert(!IsEmptyBit());

  Bit b(mMsbBit+1); M_Constructed(Bit);

  for (int i=0; i<=b.mNumWords; i++) {
    if (i<b.mNumWords) {
      if (b.mpWords[i] == cWordMax) { b.mpWords[i] = 0;      }
      else                          { b.mpWords[i]++; break; }
    }
    else {
      if ((b.mpWords[i] && cBitSetUpto[b.mMsbBit%cWordSize])
      ==                   cBitSetUpto[b.mMsbBit%cWordSize])
      {
        b.mpWords[i] = 0;
      }
      else { b.mpWords[i]++; break; }
    }
  }

  (*this) = b;

  return *this;
}

Bit& Bit::operator--(int)
{
  assert(!IsEmptyBit());

  Bit b(mMsbBit+1); M_Constructed(Bit);

  for (int i=0; i<=b.mNumWords; i++) {
    if (i<b.mNumWords) {
      if (b.mpWords[i] == 0) { b.mpWords[i] = cBitSetUpto[31]; }
      else                   { b.mpWords[i]--; break;          }
    }
    else {
      if (b.mpWords[i] == 0) { b.mpWords[i] = cBitSetUpto[b.mMsbBit%cWordSize]; }
      else                   { b.mpWords[i]--; break;                           }
    }
  }

  (*this) = b;

  return *this;
}

void Bit::operator+=(Bit &bit)
{
  assert(!IsEmptyBit());

  // In order to prevent cascading of + operator (b=b0+b1+b2) we return void.
  // Cascading causes problems because it will modify the intermediate b1 above
  // which is not our goal.
  // In order to cascade we must cast the rhs to 'unsigned', i.e.:
  //   b=(unsigned)b0+(unsigned)b1+(unsigned)b2;

  Bit b0(2 +     mMsbBit -     mLsbBit); M_Constructed(Bit);
  Bit b1(2 + bit.mMsbBit - bit.mLsbBit); M_Constructed(Bit);

  // We dont allow addition of a larger Bit operand.
  assert(b0.mMsbBit >= b1.mMsbBit);

  unsigned carry = 0;
  unsigned prev_b0_value;

  for (short i=0; i==0 || (i<b0.mNumWords && carry); i++) {
    prev_b0_value  = b0.mpWords[i];
    b0.mpWords[i] += b1.mpWords[i] + carry;

    // Check for overflow (carry), we do not check the final overflow
    // (which is the overflow beyond msb) because verilog does the same.
    if (i != b0.mNumWords-1) {
      if (b0.mpWords[i] < prev_b0_value) carry = 0;
      else                               carry = 1;
    }
  }

  *this = b0;
}

void Bit::operator-=(Bit &bit)
{
  assert(!IsEmptyBit());

  // See note for operator+ as to why we return 'void'.

  Bit b0(2 +     mMsbBit -     mLsbBit); M_Constructed(Bit);
  Bit b1(2 + bit.mMsbBit - bit.mLsbBit); M_Constructed(Bit);

  // We dont allow subtraction of a larger Bit operand.
  assert(b0.mMsbBit >= b1.mMsbBit);

  unsigned borrow = 0;

  for (short i=0; i==0 || (i<b0.mNumWords && borrow); i++) {
    if (borrow) b0.mpWords[i]--;

    if (b0.mpWords[i] < b1.mpWords[i]) {
      borrow = 1; b0.mpWords[i] = cWordMax - b1.mpWords[i] + b0.mpWords[i];
    }
    else {
      borrow = 0; b0.mpWords[i] -= b1.mpWords[i];
    }

    // Check for underflow? For now dont allow it.
    if (i == b0.mNumWords-1 && borrow) assert(0);
  }

  *this = b0;
}

Bit& Bit::operator+(unsigned u)
{
  assert(!IsEmptyBit());

  Bit b(mMsbBit+1); M_Constructed(Bit);

  unsigned prev_b_value;
  unsigned carry = 0;

  for (short i=0; i==0 || (i<b.mNumWords && carry); i++) {
    prev_b_value  = b.mpWords[i];
    b.mpWords[i] += u + carry;

    // Check for overflow (carry), we do not check the final overflow
    // (which is the overflow beyond msb) because verilog does the same.
    if (i != b.mNumWords-1) {
      if (b.mpWords[i] < prev_b_value) carry = 0;
      else                             carry = 1;
    }
  }

  *this = b;

  return *this;
}

Bit& Bit::operator-(unsigned u)
{
  assert(!IsEmptyBit());

  Bit b(1+mMsbBit-mLsbBit); M_Constructed(Bit);

  unsigned borrow = 0;

  for (short i=0; i==0 && (i<b.mNumWords && borrow); i++) {
    if (borrow) b.mpWords[i]--;

    if (b.mpWords[i] < b.mpWords[i]) {
      borrow = 1; b.mpWords[i] = cWordMax - u + b.mpWords[i];
    }
    else {
      borrow = 0; b.mpWords[i] -= u;
    }

    // Check for underflow? For now dont allow it.
    if (i == b.mNumWords-1 && borrow) assert(0);
  }

  *this = b;

  return *this;
}

Bit& Bit::operator>>(int u)
{
  if (IsEmptyBit()) return *this;

  if (u > mMsbBit-mLsbBit) {
    *this = 0;
  }
  else {
    Bit b(1+mMsbBit-mLsbBit-u); M_Constructed(Bit);

    short msb = mMsbBit; // need to save mMsbBit and mLsbBit as they will
    short lsb = mLsbBit; // be modified during the assignment operation

    b = (*this)[msb][lsb+u];

    (*this)[msb-u][lsb]   = b;
    (*this)[msb][msb-u+1] = 0;
  }
  
  return *this;
}

Bit& Bit::operator<<(int u)
{
  if (IsEmptyBit()) return *this;

  if (u > mMsbBit-mLsbBit) {
    *this = 0;
  }
  else {
    Bit b(1+mMsbBit-mLsbBit-u); M_Constructed(Bit);

    short msb = mMsbBit; // need to save mMsbBit and mLsbBit as they will
    short lsb = mLsbBit; // be modified during the assignment operation

    b = (*this)[msb-u][lsb];
    ResetBitRange();

    (*this)[msb][lsb+u]   = b;
    (*this)[lsb+u-1][lsb] = 0;
  }
  
  return *this;
}

Bit& Bit::operator[](short bit)
{
  assert(!IsEmptyBit());

  // Normal sequence is a) msb->lsb->Clear(), used to specify a range
  //                    b) msb->Clear(),      used to specify a single bit
  switch (mBitRangeState) {
    case eBitRangeState_Clear:     { mMsbBit=mLsbBit=bit;      mBitRangeState=eBitRangeState_MsbSet;    break; }
    case eBitRangeState_MsbSet:    { mLsbBit=bit;              mBitRangeState=eBitRangeState_LsbSet;    break; }
    case eBitRangeState_LsbSet:    { m2ndMsbBit=m2ndLsbBit=bit;mBitRangeState=eBitRangeState_2ndMsbSet; break; }
    case eBitRangeState_2ndMsbSet: { m2ndLsbBit=bit;           mBitRangeState=eBitRangeState_2ndLsbSet; break; }
    case eBitRangeState_2ndLsbSet: { assert(0); }
    default:                       { assert(0); }
  }
}

short Bit::IsBitSet(short bit_num) const
{
  assert(!IsEmptyBit());

  assert(bit_num < mNumBits);

  if (bit_num < mNumBits) {
    short word = bit_num>>cWordSizeLog;
    short bit  = bit_num%cWordSize;

    if (mpWords[word] & cBitSetAt[bit]) return TRUE;
  }

  return FALSE;
}

void Bit::SetBit(short bit_num)
{
  assert(!IsEmptyBit());
  assert(bit_num < mNumBits);

  if (bit_num < mNumBits) {
    short word = bit_num>>cWordSizeLog;
    short bit  = bit_num%cWordSize;

    mpWords[word] = mpWords[word] | cBitSetAt[bit];
  }
}

void Bit::BitSetAll()
{
  if (IsEmptyBit()) return;

  short i;
  short j;

  switch (mNumWords) {
  case 0:
    break;
  default:
    for (i=0; i<mNumWords-1; i++) {
      mpWords[i] = cWordMax;
    }
    // Now set bits in the last word.
    for (j=0; j<mNumBits%cWordSize; j++) {
      mpWords[i] = mpWords[i] | cBitSetAt[j];
    }
  }
}

void Bit::BitClearAll()
{
  if (IsEmptyBit()) return;

  short i;
  short j;

  switch (mNumWords) {
  case 0:
    break;
  default:
    for (i=0; i<mNumWords-1; i++) {
      mpWords[i] = 0;
    }
    // Now clear bits in the last word.
    for (j=0; j<mNumBits%cWordSize; j++) {
      mpWords[i] = mpWords[i] & !cBitSetAt[j];
    }
  }
}

void Bit::ClearBit(short bit_num)
{
  assert(!IsEmptyBit());
  assert(bit_num < mNumBits);

  if (bit_num < mNumBits) {
    short word = bit_num>>cWordSizeLog;
    short bit  = bit_num%cWordSize;

    mpWords[word] = mpWords[word] & cBitNotSetAt[bit];
  }
}

Bit& Bit::operator=(Interface& intf)
{
  intf.GetCommittedValue(*this);

  return *this;
}

void Bit::SetMsbLsb(short msb, short lsb)
{
  if (IsEmptyBit()) {
    InitBitsAndWords(msb, lsb);
  }
  else {
    assert(msb<mNumBits && lsb<mNumBits && msb>=lsb);

    mMsbBit = msb;
    mLsbBit = lsb;
  }
}

Bit::operator unsigned int()
{
  unsigned u;
  short w0 = mLsbBit>>5;
  short w1 = mMsbBit>>5;
  short width = mMsbBit - mLsbBit;

  // Check if u fits in 'width' bits.
  assert(width<=cWordSize);

  if (w0 == w1) {
    // msb and lsb are in the same word.
    u = (mpWords[w0]>>(mLsbBit%32)) & cBitSetUpto[width];
  }
  else {
    // msb and lsb are not in the same word, first assign lower word.
    short width_w0 = cWordSize - (mLsbBit%32);
    u = mpWords[w0] & (cBitSetUpto[width_w0]<<(mLsbBit%32));

    // now assign upper word.
    short width_w1 = mMsbBit%32;
    u = u | ((mpWords[w1] & cBitSetUpto[width_w1]) << width_w0);
  }

  ResetBitRange();

  return u;
}

Bit::operator short()
{
  assert(!IsEmptyBit());

  short        s;
  unsigned int v;
  
  v = (unsigned int)(*this);

  assert(v <= SHRT_MAX);

  return s=v;
}

Bit::operator int()
{
  assert(!IsEmptyBit());

  // No support yet for -ve interpretation of bits, convert from unsigned int.
  int          i;
  unsigned int v;
  
  v = (unsigned int)(*this);

  assert(v <= INT_MAX);

  return i=v;
}

Bit::operator unsigned short()
{
  assert(!IsEmptyBit());

  unsigned short s;
  unsigned int   v;
  
  v = (unsigned int)(*this);

  assert(v <= USHRT_MAX);

  return s=v;
}

Bit::operator bool()
{
  return (*this > 0);
}

Bit& Bit::SelfCopy()
{
  // CONT: Maybe we can improve the msb-lsb tracking state machine to not need
  //       SelfCopy(). We track upto 4 [short] lsb/msb index calls. Once we
  //       notice that we have self assignment then we assign as follows:
  //       0 index calls =>  b=b; // OK, NOP (do nothing)
  //       1 index call           // DO NOT ALLOW THIS, multiple cases possible
  //       2 index calls          // example 2 calls: b[3]=b[2], b[3][2] = b etc.
  //       3 index calls          // example 3 calls: b[3][2]=b[5], b[3]=b[2][5]
  //       4 index calls =>  b[5][3]=b[3][1];  // OK, compute using 3=msb/1=lsb/5=msb1/3=lsb1

  // We use 'casting to Bit' in order to save the Bit into a temp
  // this is currently needed when assigning Bit to self: b[4] = b[5];
//*gSavedBit   = *this; // FIX
  mSavedBitSet = TRUE;

  ResetBitRange();

  return *this;
}

// Currently used for Vcd dump.
void Bit::VcdPrint(FILE* fp) const
{
  assert (!IsEmptyBit()); // cannot print uninitialized bit

  bool found_non_zero_bit = FALSE;

  // Vcd prints scalars (bit width 1) and vectors differently.
  if (BitWidth() == 1) {
    fprintf(fp, "%x", mpWords[0]); // no space after value for scalars
  }
  else {
    fprintf(fp, "b"); // need leading 'b' for vectors

    for (short i=mNumWords-1; i>=0; i--) {
      if (mpWords[i]==0) continue;

      // Need to print in binary.
      for (int j=31; j>=0; j--) {
        if (mpWords[i] & cBitSetAt[j]) { found_non_zero_bit = TRUE; fprintf(fp, "1"); }
        else                           { if (found_non_zero_bit)    fprintf(fp, "0"); }
      }
    }

    if (!found_non_zero_bit) fprintf(fp, "0");
    fprintf(fp, " "); // space after value for vectors
  }
}

void Bit::Print(const char* format_str) const
{
  char buf [MAX_RECORD_LINE_LENGTH+1];

  if (format_str) fprintf(stdout, format_str, Save(buf));
  else            fprintf(stdout, "%s\n", Save(buf));
}

char* Bit::Save(char buf [MAX_RECORD_LINE_LENGTH+1]) const
{
  char hex_buf [cCharsPerWordHex+1];
  
  if (!IsEmptyBit()) {
    for (short i=mNumWords-1; i>=0; i--) {
      if (i != mNumWords-1) {
        // Lsb words.
        sprintf(hex_buf, "%x", mpWords[i]);
        strcat(buf, hex_buf);
      }
      else {
        // Msb word.
        switch (mNumBits % cCharsPerWordHex) {
          case 0: sprintf(buf, "%8x", mpWords[i]); break;
          case 1: sprintf(buf, "%1x", mpWords[i]); break;
          case 2: sprintf(buf, "%2x", mpWords[i]); break;
          case 3: sprintf(buf, "%3x", mpWords[i]); break;
          case 4: sprintf(buf, "%4x", mpWords[i]); break;
          case 5: sprintf(buf, "%5x", mpWords[i]); break;
          case 6: sprintf(buf, "%6x", mpWords[i]); break;
          case 7: sprintf(buf, "%7x", mpWords[i]); break;
          default: assert(0);
        }
      }
    }
  }
  else {
    buf[0] = '\0';
  }

  return buf;
}

// End
