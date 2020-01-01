// $RCSfile: Signal.h,v $
// $Revision: 1.5 $

#ifndef SignalClass
#define SignalClass

#include <string.h>

#include "Attribute.h"
#include "Bit.h"
#include "DBDefines.h"
#include "List.h"
#include "Simulator.h"
#include "String.h"

class Clock;
class Interface;

enum eSignalType {
  eSignalType_Input,
  eSignalType_Output,
  eSignalType_InOut,
  eSignalType_Strap,
  eSignalType_Local,
  eSignalType_None,
//eSignalType_Wire, // determine if this could be useful
  eSignalType_Invalid
};

// This class will probably just contain all Signal related information
// without any significant operations. The important operations on Signals
// are done by the friend class: Interface.
class Signal
{
  friend class Interface;// does a lot of Signal processing
  friend class Process;  // accesses mSensitiveTypeList
  friend class Scheduler;// accesses signal connection list to see if tied to clock
  friend class Vcd;      // sometimes needs to access signal connection list

public:
  // Note: Constructor is private, accessible only to class Interface.

  bool    Connect(Signal* signal);

  bool    Update(unsigned u);
  bool    Update(Bit&     bit);    // cannot use const, we reset bit state
  void    Update(Signal&  signal); // cannot use const, we reset bit state
  //
  bool    NameIs        (const char* name)       const;
  bool    BelongsToGroup(const char* group_name) const;
  //
  bool IsDrivingSignal();
  bool IsReceivingSignal();
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
  eSignalType SignalType()   const { return mSignalType; }
  Attribute*  AttributeGet() const { return mpAttribute; }
  bool        IsUpdated()    const { return mIsStrapUpdated;  }
  const char* Name()         const { return (const char*)mName; }


private:
  Bit               mBit;
  String            mName;
  String            mGroupName;
  eSignalType       mSignalType;
  const char*       mSignalFlags;
  Attribute*        mpAttribute;
  Interface*        mpInterface;
  bool              mIsModified;
  bool              mIsStrapUpdated;
  bool              mIsDrivenByClock;
  bool              mPropagateSignal;
  char              mpVcdIdentifier[4];
  short             mUpdateFailCount;

  List<Signal>        mConnectedSignalsList; // signal list that this signal connects to/from.
  List<SensitiveInfo> mSensitiveInfoList;    // processes that are sensitive to this signal.

  short               mMultipleUpdatesCount;  // track times a signal is updated during same tick.
  static const short  cMultipleUpdatesCountMax = 100;

  // Private member functions:
  Signal(const char* name, short width, const char* flags, eSignalType type, const char* group, Attribute *attr, Interface* intf, Bit* init_val=0);
  ~Signal();
};

#endif

// End
