// $RCSfile: Signal.cc,v $
// $Revision: 1.4 $

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "Signal.h"
//
#include "Interface.h"
#include "Scheduler.h"

extern Scheduler* gScheduler;
extern int        gDbg;

static bool       sDbg = 1;

M_ExternConstructDestructCountInt;

Signal::Signal(const char* name, short width, const char* flags, eSignalType type, const char* group_name, Attribute *attr, Interface *intf, Bit* init_val) :
  mName                    (name),
  mGroupName               (group_name),
  mBit                     (width),
  mSignalFlags             (flags),
  mSignalType              (type),
  mpAttribute              (attr),
  mpInterface              (intf),
  mIsModified              (FALSE),
  mUpdateFailCount         (0),
  mIsStrapUpdated          (FALSE),
  mIsDrivenByClock         (FALSE),
  mPropagateSignal         (FALSE),
  mMultipleUpdatesCount    (0)
{
  mpVcdIdentifier[0] = '\0';

  assert(width > 0);

  if (init_val) {
    mBit = *init_val;

    if (mSignalType == eSignalType_Strap) mIsStrapUpdated = TRUE;
  }

  M_UpdConstructCount(eType_Signal);
}

Signal::~Signal()
{
  // Only delete the links, the link elements will be
  // deleted in the Process class which keeps the same list.
  // The mSensitiveInfoList destructor will delete all the links.

  M_UpdDestructCount(eType_Signal);
}

bool Signal::Connect(Signal* signal)
{
  bool result = FALSE;

  if (mBit.BitWidth() == signal->mBit.BitWidth()) {
    result  = mConnectedSignalsList.Add(signal);
    result &= signal->mConnectedSignalsList.Add(this);
    if (sDbg) printf("Connecting %s with %s", Name(), signal->Name()), M_FL;

    // For now, input signal should have only one driving signal.
    // Later we may allow signal types of weak0, strong1 etc. to resolve
    // the final driven value from multiple signal sources.
    if (mSignalType == eSignalType_Input)
      assert (mConnectedSignalsList.Size() == 1);
    if (signal->mSignalType == eSignalType_Input)
      assert (signal->mConnectedSignalsList.Size() == 1);
  }
  else assert(0);

  return result;
}

bool Signal::NameIs(const char* name) const
{
  return mName == name;
}

bool Signal::BelongsToGroup(const char* name) const
{
  return mGroupName.Find(name);
}

bool Signal::IsDrivingSignal()
{
  if (mSignalType == eSignalType_Strap 
  ||  mSignalType == eSignalType_Output 
  ||  mSignalType == eSignalType_InOut)
  {
    return TRUE;
  }

  return FALSE;
}

bool Signal::IsReceivingSignal()
{
  if (mSignalType == eSignalType_Input 
  ||  mSignalType == eSignalType_InOut)
  {
    return TRUE;
  }

  return FALSE;
}

void Signal::IsDrivenByClock(bool is_driven_by_clock)
{
  mIsDrivenByClock = is_driven_by_clock;
}

bool Signal::Update(Bit& bit)
{
  // Note: Any modifications here need to be reflected in Update(unsigned u).
  // Can only assign to Outputs or In-Outs or Strap (if not updated already).

  if (mSignalType == eSignalType_Output
  ||  mSignalType == eSignalType_InOut)
  {
    if (mBit != bit) {
      mBit               = bit;
      mIsModified        = TRUE;

    }
    return TRUE;
  }
  else if (mSignalType == eSignalType_Strap)
  {
    if (! mIsStrapUpdated) {
      mBit            = bit;
      mIsModified     = TRUE;
      mIsStrapUpdated = TRUE;

      mpInterface->SetInputsUpdated();

      return TRUE;
    }
    else {
      assert(0); // only one update allowed for strap signals
    }
  }

  mUpdateFailCount++;

  return FALSE;
}

bool Signal::Update(unsigned u)
{
  // Note: Any modifications here need to be reflected in Update(Bit& b).
  // Can only assign to Outputs or In-Outs or Strap (if not updated already).

  if (mSignalType == eSignalType_Output
  ||  mSignalType == eSignalType_InOut)
  {
    if (mBit != u) {
      mBit               = u;
      mIsModified        = TRUE;

      mpInterface->SetInputsUpdated();
    }

    return TRUE;
  }
  else if (mSignalType == eSignalType_Strap)
  {
    if (! mIsStrapUpdated) {
      mBit            = u;
      mIsModified     = TRUE;
      mIsStrapUpdated = TRUE;

      mpInterface->SetInputsUpdated();

      return TRUE;
    }
    else {
      assert(0); // only one update allowed for strap signals
    }
  }

  mUpdateFailCount++;

  return FALSE;
}

void Signal::Update(Signal& signal)
{
  mBit = signal.mBit;

  // Typically Update() is called from PropagateInterfaceSignals() and the
  // propagate only happens if the source value changed. Then, if this
  // signal has input only from that source then it too must be now modified!
  mIsModified = TRUE;
}

// When an input signal is updated check if any processes are sensitive to the
// change (except for clock signals, they are anyways always scheduled by the
// Scheduler.
Process* Signal::HasSensitiveProcess()
{
  if (mSensitiveInfoList.Size()) {
    for (M_ListIterateAll(mSensitiveInfoList)) {
      SensitiveInfo* info = mSensitiveInfoList.Get();
      assert(info);

      if (mBit.IsSensitive(info->mSensitiveType, mIsModified)) {
        // Can we use mpInterface->mpOwningProcess instead of info->mpProcess?
        // A. Probably yes.
        return info->mpProcess;
      }
    }
  }

  return (Process*)0;
}

void Signal::MarkAsInputIfReversed()
{
  // Since we allow Interfaces to be created with signal directions reversed
  // > new Interface(name, intf_desc, eIntfModifier_SignalDirInvert);
  // an input that is meant to be a clock input will become an output. These
  // are the only signals we actually dont want to invert. The 'reversing'
  // logic does not know which inputs will be connected to a clock and we dont
  // want to enforce a particular signal naming scheme (like Clk, clk, clock
  // etc.) or create a new signal type (like eSignalType_Clock). Therefore we
  // simply fix the signal direction once it is connected to a clock (but only
  // if it was reversed).

  if (mSignalType == eSignalType_Output
  &&  mpInterface->InterfaceModifierGet() == eIntfModifier_SignalDirInvert)
  {
    mSignalType = eSignalType_Input;
  }
}

// End
