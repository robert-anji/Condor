// $RCSfile: Interface.h,v $
// $Revision: 1.3 $

#ifndef InterfaceClass
#define InterfaceClass

#include "List.h"
//#include "Module.h"
#include "Signal.h"
#include "String.h"
#include "Simulator.h"

class Attribute;
class Process;
class Table;
class View;
class ViewIterator;

enum eSignalSaveCount {
  eSignalSaveCountDefault = 5
};

enum eInputsModifiedCount {
  eInputsModifiedCount_None,
  eInputsModifiedCount_AtLeastOne,
  eInputsModifiedCount_All
};

enum eInterfaceUpdateMode {
  eIntfUpdMode_InputsAny,
  eIntfUpdMode_InputsAll,
  eIntfUpdMode_InputsNone
};

enum eInterfaceModifier {
  eIntfModifier_None,
  eIntfModifier_SignalDirInvert,
  eIntfModifier_SignalDirMonitorMode,
  eIntfModifier_Invalid
};

enum eWhichSignalPropagates {
  eLeftSignalPropagates,
  eRightSignalPropagates,
  eBothSignalsPropagate,
  eNoSignalPropagates
};

enum eInterfaceConnectType {
  eInterfaceConnectType_Peer,
  eInterfaceConnectType_PassThru
};

struct InterfaceDescriptionStruct {
  int            signal_pos;
  const char*    signal_name;
  short          signal_width;
  const char*    signal_flags;
  eSignalType    signal_type;
  const char*    signal_group;
};
typedef InterfaceDescriptionStruct InterfaceDescription;

class Interface
{
  friend class Scheduler;
  friend class Vcd;
  friend class Module;

public:
  Interface(const char* name, const InterfaceDescription* intf_desc, eInterfaceModifier mod=eIntfModifier_None);
  ~Interface();

  // Create sub-interfaces.
  Interface* SubInterfaceGet(const char* group_name);

  // Control functions:
  bool  HistoryDepth(short count);
  bool  SetOwningProcess(Process* process);

  // Compare functions:
  bool operator==(Bit& bit);
//bool operator!=(Bit& bit);
//bool operator> (Bit& bit);
//bool operator< (Bit& bit);
//bool operator<=(Bit& bit);
//bool operator>=(Bit& bit);
  bool operator==(unsigned u);

  // Assigning to an Interface will assign the current 'mSignalPos' signal.
  bool operator=(Interface& intf);

  // Set and retrieve Interface signal values (read returns committed values).
  bool         operator=(Bit&       bit);    // set (write) output signal
  bool         operator=(Signal&    signal); // set (write) input  signal
  bool         operator=(unsigned   u);      // set (write) input  signal
  // bool Bit::operator=(Interface& intf);   // get (read) signal
  // Helper functions for this are:
  Interface& operator[](int signal_pos);
  Interface& operator[](const char* signal_name);
  bool       GetCommittedValue(Bit& bit);  // used when: bit = intf[signal];
  // Thus use:
  //   intf[signal_name] = bit;               // to write a signal
  //   intf[signal_name] = signal;            //         "
  //   intf[signal_name] = u;                 //         "        , or
  //   bit               = intf[signal_name]; // to read a signal

  Signal* SignalGet(const char* name);
  void    SetInputsUpdated() { mInputsUpdated = TRUE; }

  // Connect two signals together.
  void Connect(Signal* signal1, Signal* signal2, eInterfaceConnectType connect_type, bool& connect_error);

  // Query functions.
  bool          IsSubInterface() const { return (mpParentInterface != 0);    }
  const char*   Name()           const { return (const char*)mInterfaceName; }
  bool          IsSynchronous()  const { return mIsSynchronous;              }
  //
  eInterfaceModifier InterfaceModifierGet() const { return mIntfModifier; }

  const InterfaceDescription* mpInterfaceDescription;
  Process* GetOwningProcess() const { return mpOwningProcess; }

  operator bool();

private:
  List<Signal>  mSignalList; // stores 'new' values of interface signals
  Signal**      mppSignals;
  bool*         mpSubIntfSignalPosValid;
  String        mInterfaceName;
  Process*      mpOwningProcess;
  //
  Table*        mpSignalsTable;
  View*         mpSignalsView; // stores 'current' values of interface signals
  ViewIterator* mpSignalsViewIter;
  short         mSignalTotal;
  short         mSignalPos;
  unsigned      mSignalSaveCount;
  unsigned      mSignalCommitCount;
  Interface*    mpParentInterface;
  bool          mNeedsCommit;
  bool          mIsSynchronous;
  bool          mInputsUpdated;

  const Attribute*     mpSignalAttr;
  eInterfaceUpdateMode mIntfUpdateMode; //check if we still have need for this var
  eInterfaceModifier   mIntfModifier;

  // Private member functions:
  // Sub-interface constructor.
  Interface(const char* intf_name, Interface* parent_intf);
  //
  void SetAsSynchronous() { mIsSynchronous = TRUE; }
  //
  // Propagate the interface signals to connected signals as required.
  void  PropagateInterfaceSignals(bool& all_ok, List<Process>& proc_to_be_sched); //
  eWhichSignalPropagates WhichSignalPropagates(Signal* signal1, Signal* signal2, eInterfaceConnectType type) const;
  short Commit(); // reserved for friend class: Scheduler
  //
  void NeedsCommit(bool flag) { mNeedsCommit = flag; }
  bool NeedsCommit()          { return mNeedsCommit; }

  // Temporary members (avoids repeated constructor/destructor calls).
  Bit  impSignalBit;
};

#endif

// End
