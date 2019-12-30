// $RCSfile: TestTemplate.cc,v $
// $Revision: 1.1 $

#include <stdio.h>

#include "Global.h"
#include "Database.h"
#include "Enum.h"
#include "List.h"
#include "Module.h"
#include "Process.h"
#include "Simulator.h"
#include "Vcd.h"
//
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
void process_fn1(ProcessExecuteReason& rsn, Process* proc);

void test_timer();

int main(int argc, char* argv[])
{
  for (unsigned u=0; u<eType_Max; u++) { gConstructCount[u]=gDestructCount[0]=0; }

  test_timer();

  show_memory_usage();

  return 1;
}

void test_timer()
{
  gDB = new Database("./", "FirstVerifyRightDB");

  gSimulator = new Simulator();

  Module* parent_module;

  Module* mod_top = new Module("Module_TestMailbox", parent_module=0);
  gVcd            = new Vcd(); // keep Vcd handle handy in case dumping is turned on.

  // Add Test Code HERE
  new Process("TestProcess1", mod_top, process_fn1);
  // End Test Code

  gSimulator->SimulationTimeMax(400*1);
  gSimulator->Start(mod_top);

  delete mod_top;
  delete gVcd;
  delete gSimulator;
  delete gDB;
}

Timer* tmr1;
Timer* tmr2;

void process_fn1(ProcessExecuteReason& rsn, Process* proc)
{
  if (rsn.ReasonGet() == eExecuteReason_Initial) {
    tmr1 = new Timer("TestTimer1", gModuleTop);
    tmr1->Start(100, eTimeScale_ns, proc);

    tmr2 = new Timer("TestTimer2", gModuleTop);
    tmr2->Start(200, eTimeScale_ns, proc);

    proc->Wait(tmr2); // this should cause us to miss tmr1

    printf("Hello there!\n");
  }

  if (rsn.ReasonGet() == eExecuteReason_TimerDone) {
    printf("Timer completed at %u\n", gSimTime);
  }
}

#include "MemoryUsage.cc"

// End
