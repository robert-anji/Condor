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

void test_testname();

int main(int argc, char* argv[])
{
  for (unsigned u=0; u<eType_Max; u++) { gConstructCount[u]=gDestructCount[0]=0; }

  test_testname();

  show_memory_usage();

  return 1;
}

void test_testname()
{
  gDB = new Database("./", "FirstVerifyRightDB");

  gSimulator = new Simulator();

  Module* parent_module;

  Module* mod_top = new Module("Module_TestMailbox", parent_module=0);
  gVcd            = new Vcd(); // keep Vcd handle handy in case dumping is turned on.

  // Add Test Code HERE
  // End Test Code

  gSimulator->SimulationTimeMax(400*1);
  gSimulator->Start(mod_top);

  delete mod_top;
  delete gVcd;
  delete gSimulator;
  delete gDB;
}

#include "MemoryUsage.cc"

// End
