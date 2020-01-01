// $RCSfile: AsyncToggle.cc,v $
// $Revision: 1.1 $

#include <stdio.h>

#include "Global.h"
#include "Database.h"
#include "Enum.h"
#include "Interface.h"
#include "List.h"
#include "Module.h"
#include "Process.h"
#include "Simulator.h"
#include "Vcd.h"
//
#include "Bit.h"
#include "Timer.h"

int            gConstructCount[eType_Max];
int            gDestructCount[eType_Max];
Database*      gDB;
Simulator*     gSimulator;
SimulationTime gSimTime;
Vcd*           gVcd;
bool           gDbg;
extern Module* gModuleTop;
//
void show_memory_usage();

// Test Code
void process_tff(ProcessExecuteReason& rsn, Process* proc);
void process_stimulus(ProcessExecuteReason& rsn, Process* proc);

const char* const cTFF_D  = "TFF_D";
const char* const cTFF_Q  = "TFF_Q";
//
enum eTFF {
  eTFF_D,
  eTFF_Q,
  eTFF_END
};
//
InterfaceDescription Interface_TFF[] = {
  eTFF_D,  cTFF_D, 1, eSignalType_Input,  "TFF",
  eTFF_Q,  cTFF_Q, 1, eSignalType_Output, "TFF",
  eTFF_END,  ""  , 0, eSignalType_Invalid,   ""
};
// End Test Code

void test_toggle_flipflop();

int main(int argc, char* argv[])
{
  for (unsigned u=0; u<eType_Max; u++) { gConstructCount[u]=gDestructCount[0]=0; }

  test_toggle_flipflop();

  show_memory_usage();

  printf("ROB: main done\n");

  return 1;
}

static Timer* gTimer;

void test_toggle_flipflop()
{
  gDB = new Database("./", "FirstVerifyRightDB");

  gSimulator = new Simulator();

  Module* parent_module;

  Module* mod_top = new Module("TestModule", parent_module=0);
  gVcd            = new Vcd(); // keep Vcd handle handy in case dumping is turned on.

  // Add Test Code HERE
  Process* p1 = new Process("AsyncToggleFF", mod_top, process_tff);
  Process* p2 = new Process("Stimulus",      mod_top, process_stimulus);
  printf("AsyncToggleFF=%p\n", p1);
  printf("Stimulus=%p\n", p2);

  Interface* intf_tff  = new Interface("Interface_TFF.0", Interface_TFF);
  Interface* intf_stim = new Interface("Interface_Stim",  Interface_TFF, eIntfModifier_SignalDirInvert);

  p1->InterfaceSet(intf_tff);
  p2->InterfaceSet(intf_stim);

  mod_top->ConnectInterfaceGroup(intf_tff, intf_stim,  "TFF");

  gTimer = new Timer("StimulusTimer", mod_top);
  printf("Timer=%p\n", gTimer);
  // End Test Code

  gSimulator->SimulationTimeMax(400*1);
  gSimulator->Start(mod_top);

  delete mod_top;
  delete gVcd;
  delete gSimulator;
  delete gDB;
}

// Test Code
void process_tff(ProcessExecuteReason& rsn, Process* proc)
{
  if (rsn.ReasonGet() == eExecuteReason_Initial) {
    // Do nothing.
    printf("ROB: TFF:Initial\n");
  }
  else {
    Interface& intf = *(proc->InterfaceGet());

    Bit b = intf[eTFF_D];
    intf[eTFF_Q] = ! b;
    printf("ROB TFF: D=");
    b.Print();
    printf("\n");
  }
}

void process_stimulus(ProcessExecuteReason& rsn, Process* proc)
{
  static int count;

  Interface& intf = *(proc->InterfaceGet());

  if (rsn.ReasonGet() == eExecuteReason_Initial) {
    printf("ROB: ToggleFF=Initial\n");

    intf[eTFF_D] = b_0;

    gTimer->Start(100, eTimeScale_ns, proc);
  }
  //
  if (rsn.ReasonGet() == eExecuteReason_TimerDone) {
    printf("ROB: ToggleFF=TimerDone\n");
    intf[eTFF_D] = count&1;
    count++;

    if (count < 10)
      gTimer->Start(100, eTimeScale_ns, proc);
  }
}
// End Test Code

#include "MemoryUsage.cc"

// End
