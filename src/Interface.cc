// $RCSfile: Interface.cc,v $
// $Revision: 1.5 $

#include <assert.h>
#include <string.h>

#include "Interface.h"
//
#include "Database.h"
#include "Global.h"
#include "Module.h"
#include "Process.h"
#include "Record.h"
#include "Scheduler.h"
#include "Signal.h"
#include "Table.h"
#include "Vcd.h"
#include "View.h"

extern Database*  gDB;
extern Scheduler* gScheduler;
extern Vcd*       gVcd;
extern int        gDbg;
//
M_ExternConstructDestructCountInt;

// Create an interface consisting of a signal list from an InterfaceDescription
// and also create a table with the same signals as the table attributes.
// The signals in the signal list are used to connect them to other signals and
// also to hold the signal values. The scheduler will commit the signal values
// to the table at the appropriate time. The interface table is needed so that
// interface values can be queried globally.
//
Interface::Interface(const char* intf_name, const InterfaceDescription* intf_desc, eInterfaceModifier modifier) :
  mInterfaceName         (intf_name),
  mpInterfaceDescription (intf_desc),
  mIntfModifier          (modifier),
  mppSignals             (0),
  mpSubIntfSignalPosValid(0),
  mpOwningProcess        (0),
  mSignalSaveCount       (eSignalSaveCountDefault),
  mSignalCommitCount     (0),
  mpParentInterface      (0),
  mpSignalsTable         (0),
  mpSignalsView          (0),
  mpSignalsViewIter      (0),
  mSignalTotal           (0),
  mSignalPos             (SHRT_MAX),
  mIntfUpdateMode        (eIntfUpdMode_InputsAll),
  mpSignalAttr           (0),
  mNeedsCommit           (FALSE),
  mIsSynchronous         (FALSE),
  mInputsUpdated         (FALSE)
{
  assert(mpInterfaceDescription);
  assert(mSignalSaveCount);

  // Cannot use 'Auto-Commit', table needs manual commit by the Scheduler.
  int tbl_open_flags =  eTableOpenFlags_Default;

  mpSignalsTable = gDB->TableOpen(intf_name,  tbl_open_flags);

  mpSignalsView  = mpSignalsTable->ViewCreate("AllSignals",  eRecPerm_All);

  assert(mpSignalsTable);
  assert(mpSignalsView);

  mpSignalsViewIter  = mpSignalsView->ViewIteratorNew();
  assert(mpSignalsViewIter);

  if (!mpSignalsTable->ViewEnable(mpSignalsView, TRUE)) assert(0);
  mpSignalsView->RecordAddDynamic();

  // Add all the signals from the interface description.
  bool done = FALSE;

  const InterfaceDescription* desc     = mpInterfaceDescription;
  Bit*                        init_val = 0;

  // Add signal to list and as attributes to the interface table.
  // TBD: Dont allow more than one clock in an interface.
  for (int i=0; !done; i++) {
    if (desc[i].signal_width==0) { done=TRUE; mSignalTotal=i; break; }

    Attribute* attr;
    const char* name  = desc[i].signal_name;
    short       width = desc[i].signal_width;
    const char* group = desc[i].signal_group;
    const char* flags = desc[i].signal_flags;
    eSignalType type  = desc[i].signal_type;

    switch (modifier) {
    case eIntfModifier_None:
      break;          // leave 'type' unchanged.
    case eIntfModifier_SignalDirInvert:
      // Reverse only if the signal has the 'Reversible' flag set.
      if (strstr(flags, "R")) {
        if      (type == eSignalType_Input)  type = eSignalType_Output;
        else if (type == eSignalType_Output) type = eSignalType_Input;
      }
      // unchanged: _InOut, _Strap, _None, _Invalid.
      break;
    case eIntfModifier_SignalDirMonitorMode:
      type = eSignalType_Input;
      break;
    default: assert(0);
    }

    if (init_val) {
      Value v; M_Constructed(Value);

      v = *init_val;
      attr = mpSignalsTable->AttributeAdd(name, eValueBit, &v, width);
    }
    else {
      attr = mpSignalsTable->AttributeAdd(name, eValueBit, (Value*)0, width);
    }

    // Check that signal_pos in desc is set correctly otherwise
    // retrieving a signal by position will not work.
    assert (desc[i].signal_pos == i);
    assert (desc[i].signal_pos == attr->PositionGet());

    // Now we are ready to create the Signal.
    Signal* signal = new Signal(name, width, flags, type, group, attr, this, init_val);
    assert(signal);

    mSignalList.Add(signal, eListAddPosition_Last);

    assert(mSignalList.Size() != SHRT_MAX);
  }

  if (mSignalTotal) {
    mppSignals = new Signal* [mSignalTotal];

    short i = 0;

    for (MacroListIterateAll(mSignalList)) {
      Signal* signal = mSignalList.Get();
      assert(signal);

      mppSignals[i++] = signal;
    }
  }

  M_UpdConstructCount(eType_Interface);
}

// Constructor for sub-interfaces.
Interface::Interface(const char* intf_name, Interface* parent_intf) :
  mInterfaceName         (intf_name),
  mpInterfaceDescription (0),
  mIntfModifier          (parent_intf->mIntfModifier),
  mppSignals             (parent_intf->mppSignals),
  mpSubIntfSignalPosValid(0),
  mpOwningProcess        (0),
  mSignalSaveCount       (eSignalSaveCountDefault),
  mSignalCommitCount     (0),
  mpParentInterface      (parent_intf),
  mpSignalsTable         (0),
  mpSignalsView          (0),
  mpSignalsViewIter      (0),
  mSignalTotal           (parent_intf->mSignalTotal),
  mSignalPos             (SHRT_MAX),
  mIntfUpdateMode        (eIntfUpdMode_InputsAll),
  mpSignalAttr           (0),
  mNeedsCommit           (FALSE),
  mIsSynchronous         (parent_intf->mIsSynchronous),
  mInputsUpdated         (FALSE)
{
  assert(mSignalSaveCount);

  M_UpdConstructCount(eType_Interface);
}

Interface::~Interface()
{
  M_ListDeleteAll(mSignalList, Signal);

  if (mpSignalsTable) {
    if (mpSignalsView) {
      if (mpSignalsViewIter) {
        mpSignalsView->ViewIteratorFree(mpSignalsViewIter); mpSignalsViewIter = 0;
      }

      mpSignalsTable->ViewDelete(mpSignalsView); mpSignalsView = 0;
    }

    gDB->TableClose(mpSignalsTable); mpSignalsTable = 0;
  }

  if (! IsSubInterface()) {
    if (mppSignals) {
      delete [] mppSignals; mppSignals = 0;
    }
  }

  if (mpSubIntfSignalPosValid) { delete [] mpSubIntfSignalPosValid; mpSubIntfSignalPosValid=0; M_UpdDestructCountN(eType_Bool, mSignalTotal); }

  M_UpdDestructCount(eType_Interface);
}

// To avoid passing in the entire interface to a process when it actually only
// requires a part of them and to make it easier to connect interfaces to-
// gether, we use the concept of a sub-interface.
// A sub-interface contains only a subset of the signals from the 'parent'
// interface. Also, it does not need its own interface table because all signals
// can already be queried from the parent table.
Interface* Interface::SubInterfaceGet(const char* group_name)
{
  Interface* sub_intf = 0;

  assert (! IsSubInterface()); // no support for sub-interface of sub-interface

  sub_intf = new Interface(group_name, this); M_UpdConstructCount(eType_Interface);
  assert(sub_intf);

  if (mSignalTotal) {
    mpSubIntfSignalPosValid = new bool [mSignalTotal]; M_UpdConstructCountN(eType_Bool, mSignalTotal);
    assert (mpSubIntfSignalPosValid);
  }

  unsigned total_count = 0;

  for (MacroListIterateAll(mSignalList)) {
    Signal* signal = mSignalList.Get();
    assert(signal);

    // mSignalList is used for Signal access via attribute pointer.
    if (signal->BelongsToGroup(group_name)) {
      sub_intf->mSignalList.Add(signal);
      mpSubIntfSignalPosValid[total_count] = TRUE;
    }
    else
      mpSubIntfSignalPosValid[total_count] = FALSE;

    total_count++;
  }

  return sub_intf;
}

Interface& Interface::operator[](int signal_pos)
{
  mSignalPos = signal_pos;

  return *this;
}

Interface& Interface::operator[](const char* signal_name)
{
  mpSignalAttr = mpSignalsView->AttributeFind(signal_name);

  assert(mpSignalAttr);

  return *this;
}

bool Interface::operator=(Interface& intf)
{
  assert(intf.mSignalPos < intf.mSignalTotal);

  bool res = (*this = (intf.mppSignals[mSignalPos])->mBit);

  intf.mSignalPos = SHRT_MAX;

  return res;
}

// Use this to set a signal that is an output/inout/strap.
bool Interface::operator=(Bit& bit)
{
  if (mpSignalAttr) {
    // Access is by name in the attr.
    for (MacroListIterateAll(mSignalList)) {
      Signal* signal = mSignalList.Get();
      assert(signal);

      if (signal->NameIs(mpSignalAttr->Name())) {
        if (!signal->Update(bit)) assert(0);

        mpSignalAttr = 0; // clear it

        return TRUE;
      }
    }
  }
  else {
    assert(mSignalPos < mSignalTotal);

    if (IsSubInterface()) assert(mpSubIntfSignalPosValid[mSignalPos]);

    if (! ((mppSignals[mSignalPos])->Update(bit))) assert(0);

    mSignalPos = SHRT_MAX;

    return TRUE;
  }

  assert(0);
}

bool Interface::operator=(unsigned u)
{
  if (mpSignalAttr) {
    for (MacroListIterateAll(mSignalList)) {
      Signal* signal = mSignalList.Get();
      assert(signal);

      if (signal->NameIs(mpSignalAttr->Name())) {
        if (!signal->Update(u)) assert(0);

        mpSignalAttr = 0; // clear it

        return TRUE;
      }
    }
  }
  else {
    assert(mSignalPos < mSignalTotal);

    if (IsSubInterface()) assert(mpSubIntfSignalPosValid[mSignalPos]);

    if (! mppSignals[mSignalPos]->Update(u))
      assert(0);

    mSignalPos = SHRT_MAX;
  }

  return FALSE;
}

bool Interface::operator==(Bit& bit)
{
  if (mpSignalAttr) {
    for (MacroListIterateAll(mSignalList)) {
      Signal* signal = mSignalList.Get();
      assert(signal);

      if (signal->NameIs(mpSignalAttr->Name()))
        return *signal == bit;
    }
  }
  else {
    assert(mSignalPos < mSignalTotal);

    if (IsSubInterface()) assert(mpSubIntfSignalPosValid[mSignalPos]);

    short pos  = mSignalPos;
    mSignalPos = SHRT_MAX;

    return *mppSignals[pos] == bit;

  }

  return FALSE;
}

bool Interface::operator==(unsigned u)
{
  if (mpSignalAttr) {
    for (MacroListIterateAll(mSignalList)) {
      Signal* signal = mSignalList.Get();
      assert(signal);

      if (signal->NameIs(mpSignalAttr->Name()))
        return *signal == u;
    }
  }
  else {
    assert(mSignalPos < mSignalTotal);

    if (IsSubInterface()) assert(mpSubIntfSignalPosValid[mSignalPos]);

    short pos  = mSignalPos;
    mSignalPos = SHRT_MAX;

    return *mppSignals[pos] == u;
  }

  return FALSE;
}

// Use this to set a signal that is an input/inout.
bool Interface::operator=(Signal& signal)
{
  for (MacroListIterateAll(mSignalList)) {
    Signal* signal = mSignalList.Get();
    assert(signal);

    if (signal->NameIs(mpSignalAttr->Name())) {
      signal->Update(*signal);

      mpSignalAttr = 0; // clear it

      return TRUE;
    }
  }

  return FALSE;
}

Interface::operator bool()
{
  if (mpSignalAttr) {
    for (MacroListIterateAll(mSignalList)) {
      Signal* signal = mSignalList.Get();
      assert(signal);

      if (signal->NameIs(mpSignalAttr->Name())) {
        return signal->mBit != 0;
      }
    }
  }
  else {
    assert(mSignalPos < mSignalTotal);

    if (IsSubInterface()) assert(mpSubIntfSignalPosValid[mSignalPos]);

    short pos  = mSignalPos;
    mSignalPos = SHRT_MAX;

    return (*mppSignals[pos]).mBit != 0;
  }
}

void Interface::Connect(Signal* signal1, Signal* signal2, eInterfaceConnectType connect_type, bool& connect_error)
{
  MacroAssert2(signal1, signal2)

// Phased out but can be used to confirm connect_type
// is correct if owning process is set.
//Module* mod1 = signal1->mpInterface->mpOwningProcess->GetOwningModule();
//Module* mod2 = signal2->mpInterface->mpOwningProcess->GetOwningModule();
//
//eModuleRelation mod_relation;
//
//if      (mod1->IsChildModuleOf(mod2)) mod_relation = eModule1IsChild;
//else if (mod2->IsChildModuleOf(mod1)) mod_relation = eModule2IsChild;
//else                                  mod_relation = eInterModule;

  // First check for invalid connections:
  eWhichSignalPropagates w_prop = WhichSignalPropagates(signal1, signal2, connect_type);

  assert (w_prop != eNoSignalPropagates);
  
  if (w_prop==eLeftSignalPropagates  || w_prop==eBothSignalsPropagate) signal1->mPropagateSignal = TRUE;
  if (w_prop==eRightSignalPropagates || w_prop==eBothSignalsPropagate) signal2->mPropagateSignal = TRUE;

  bool result = FALSE;

  if (signal1->mBit.BitWidth() == signal2->mBit.BitWidth()) {
    result  = signal1->mConnectedSignalsList.Add(signal2);
    result &= signal2->mConnectedSignalsList.Add(signal1);
    if (gDbg) printf("Connecting %s.%s with %s.%s",
     signal1->mpInterface->Name(), signal1->Name(), signal2->mpInterface->Name(), signal2->Name()), M_FL;

    // For now, input signal should have only one driving signal if peer-to-peer
    // connection. Later we may allow signal types of weak0, strong1 etc. to
    // resolve the final driven value from multiple signal sources.
    if (connect_type == eInterfaceConnectType_Peer) {
      if (signal1->mSignalType == eSignalType_Input)
        assert (signal1->mConnectedSignalsList.Size() == 1);
      if (signal2->mSignalType == eSignalType_Input)
        assert (signal2->mConnectedSignalsList.Size() == 1);
    }
  }
  else assert(0);

  if (! result) connect_error = TRUE;
}

bool Interface::HistoryDepth(short count)
{
  mSignalSaveCount = count;

  assert(mSignalSaveCount);

  return TRUE;
}

bool Interface::SetOwningProcess(Process* process)
{
  // We dont allow an existing owning process to be overriden
  // as this may catch some user setup errors.
  if (! mpOwningProcess) mpOwningProcess = process;
  else                   assert(0);

  return TRUE;
}

// Copy the value in each signal (input/output) to the interface table & commit.
short Interface::Commit()
{
  Record* old_rec    = 0;
  Record* commit_rec = 0;

  if (mSignalCommitCount < mSignalSaveCount) {
    bool allow_immediate_val_update;
    commit_rec = mpSignalsView->RecordNew(allow_immediate_val_update=TRUE);
    
    if (mSignalCommitCount > 0)
      old_rec = mpSignalsViewIter->RecordSearchLast();
  }
  else {
    if (mSignalCommitCount % mSignalSaveCount == 0) {
      old_rec = mpSignalsViewIter->RecordSearchLast();
      mpSignalsViewIter->RecordSearchFirst();
    }
    else {
      old_rec = mpSignalsViewIter->RecordCurrentGet();

      if (!mpSignalsViewIter->RecordSearchNext())
        mpSignalsViewIter->RecordSearchFirst();
    }

    commit_rec = mpSignalsViewIter->RecordCurrentGet();
  }
  assert(commit_rec);

  for (MacroListIterateAll(mSignalList)) {
    Signal* signal = mSignalList.Get();
    assert(signal);

    Attribute* attr = signal->AttributeGet();
    assert(attr);

    (*commit_rec)[attr] = signal->mBit;

    // If dump is enabled then we dump only signal outputs, inputs not necessary.
    if (gVcd->DumpIsOn() && signal->IsDrivingSignal()) {
      if (old_rec==0 || (*commit_rec)[attr]!=(*old_rec)[attr]) {
        gVcd->DumpValueChange(signal->mpVcdIdentifier, &signal->mBit);
      }
    }

    if (gDbg) printf("Dbg: %s->%s=", Name(), signal->Name());
    if (gDbg) signal->mBit.Print("%s\n");
  }

  if (mSignalCommitCount < mSignalSaveCount) {
    // TBD: Add feature to allow us to Add and Commit in a single step.
    mpSignalsView->RecordSearchLast();
    if (mpSignalsView->TranRecordAdd(commit_rec)) mNeedsCommit = FALSE;

    gDB->DBTransactionsCommitView(mpSignalsView);
    mpSignalsViewIter->RecordSearchLast();
  }

  mSignalCommitCount++;

  return 1; // committed one transaction
}

// Propagate new values (stored in mSignalList) of the interface outputs
// to their connected inputs.
void Interface::PropagateInterfaceSignals(bool& all_ok, List<Process>& proc_to_be_sched_list)
{
  for (MacroListIterateAll(mSignalList)) {
    Signal* signal = mSignalList.Get();
    assert(signal);

    if (signal->mPropagateSignal && signal->mIsModified) {
      for (MacroListIterateAll(signal->mConnectedSignalsList)) {
        Signal* connected_signal = signal->mConnectedSignalsList.Get();
        assert(connected_signal);

      //if (gDbg) printf("Propagate %s to %s\n", signal->Name(), connected_signal->Name());
        connected_signal->Update(*signal);
        if (Process* p=connected_signal->HasSensitiveProcess()) {
          // Sync proc's will be scheduled automatically based on their clock.
          if (p->ProcessType() != eProcessType_Synchronous) {
          // Async processes can get scheduled multiple times within a delta
          // cycle but we require only one execution per delta cycle. To handle
          // this we can avoid duplicate entries in the 'schedule' list or later
          // check if an async processes has already executed in the current
          // delta cycle (maybe faster than linearly scanning the list).

          //if (! proc_to_be_sched_list.Exists(p)) // use delta cycle approach
            proc_to_be_sched_list.Add(p);
          }
        }
      }
    }
  }
}

Signal* Interface::SignalGet(const char* signal_name)
{
  for (MacroListIterateAll(mSignalList)) {
    Signal* signal = mSignalList.Get();
    assert(signal);

    if (signal->NameIs(signal_name))
      return signal;
  }

  return 0;
}

bool Interface::GetCommittedValue(Bit& bit)
{
  Record* rec = mpSignalsView->RecordSearchLast();

  if (rec) {
    if (mpSignalAttr) {
      rec->ValueCopyGet(mpSignalAttr, bit);

      mpSignalAttr = 0; // clear it

      return TRUE;
    }
    else {
      assert(mSignalPos < mSignalTotal);

      rec->ValueCopyGet(mSignalPos, bit);

      mSignalPos = SHRT_MAX;

      return TRUE;
    }
  }

  return FALSE;
}

// Returns which signal should propagate its value.
// When connecting peer-to-peer it is simply the eSignalType_Output that
// propagates, however when connecting pass-thru (parent module input connects
// to child module input or child module output connects to parent module
// output) then we need to handle it specially.
// For now this function assumes that sig1 belongs to the parent and sig2 to
// the child.
eWhichSignalPropagates Interface::WhichSignalPropagates(Signal* sig1, Signal* sig2, eInterfaceConnectType connect_type) const
{
  eWhichSignalPropagates propagate = eNoSignalPropagates;

  switch (connect_type) {
    case eInterfaceConnectType_Peer:
      if      (sig1->IsDrivingSignal()   && sig2->IsReceivingSignal()) propagate=eLeftSignalPropagates;
      else if (sig2->IsDrivingSignal()   && sig1->IsReceivingSignal()) propagate=eRightSignalPropagates;
      else if (sig1->IsDrivingSignal()   && sig2->IsDrivingSignal())   propagate=eBothSignalsPropagate;
    break;
    //
    case eInterfaceConnectType_PassThru:
      if      (sig2->IsDrivingSignal()   && sig1->IsDrivingSignal())   propagate=eRightSignalPropagates;
      else if (sig1->IsReceivingSignal() && sig2->IsReceivingSignal()) propagate=eLeftSignalPropagates;
    break;
    //
    default: assert(0);
  }
  //case eInterModule:
  //  if      (sig1->IsDrivingSignal()   && sig2->IsReceivingSignal()) propagate=eLeftSignalPropagates;
  //  else if (sig2->IsDrivingSignal()   && sig1->IsReceivingSignal()) propagate=eRightSignalPropagates;
  //  else if (sig1->IsDrivingSignal()   && sig2->IsDrivingSignal())   propagate=eBothSignalsPropagate;
  //  break;
  //case eModule1IsChild:
  //  if      (sig1->IsDrivingSignal()   && sig2->IsDrivingSignal())   propagate=eLeftSignalPropagates;
  //  else if (sig2->IsReceivingSignal() && sig1->IsReceivingSignal()) propagate=eRightSignalPropagates;
  //  break;
  //case eModule2IsChild:
  //  if      (sig2->IsDrivingSignal()   && sig1->IsDrivingSignal())   propagate=eRightSignalPropagates;
  //  else if (sig1->IsReceivingSignal() && sig2->IsReceivingSignal()) propagate=eLeftSignalPropagates;
  //  break;

  // return which signal should have 'propagate' set.
  return propagate;
}

// End
