// $RCSfile: Flop.h,v $
// $Revision: 1.5 $

#ifndef FlopClass
#define FlopClass

#include <string.h>

// Flops are hardware elements that propagate their inputs to their outputs at a
// clock edge (i.e they are synchronous). Which clock edge is used depends on
// the owning process (i.e. they should be used within a synchronous process).
// Q. Mechanism to set the vcd identifier.
// Q. Should we implement autosizing?
class Flop
{
public:
  Flop(const char* name, short width, const char* flags, eFlopType type, const char* group, Attribute *attr, Interface* intf, Bit* init_val=0);
  ~Flop();

  bool    Update(unsigned u);
  bool    Update(Bit&     bit);    // cannot use const, we reset bit state
  void    Update(Flop&  signal); // cannot use const, we reset bit state
  //
  bool    NameIs        (const char* name)       const;
  bool    BelongsToGroup(const char* group_name) const;
  //
  bool IsDrivingFlop();
  bool IsReceivingFlop();
  void IsDrivenByClock(bool is_driven_by_clock);
  //
  bool operator==(Bit& bit)   { return mBit == bit; }
  bool operator==(unsigned u) { return mBit == u;   }
  //
  short   Width() const { return mBit.BitWidth(); }
  //
  Process*  HasSensitiveProcess();
  //
  void  VcdIdentifierSet(char* code) { strcpy(mpVcdIdentifier, code); }
  char* VcdIdentifierGet()           { return mpVcdIdentifier;        }

  void MarkAsInputIfReversed();      // sometimes clk input gets reversed, fix

  // Query functions:
  eFlopType FlopType()   const { return mFlopType; }
  Attribute*  AttributeGet() const { return mpAttribute; }
  bool        IsUpdated()    const { return mIsStrapUpdated;  }
  const char* Name()         const { return (const char*)mName; }


private:
  Bit               mBit;
  String            mName;
  String            mGroupName;
  Interface*        mpInterface;
  bool              mIsModified;
  char              mpVcdIdentifier[4];
  short             mUpdateFailCount;

  short               mMultipleUpdatesCount;  // track times a signal is updated during same tick.
  static const short  cMultipleUpdatesCountMax = 100;
};

#endif

// End
