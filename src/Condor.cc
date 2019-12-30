// $RCSfile: Condor.cc,v $
// $Revision: 1.5 $

#include <assert.h>
#include <stdlib.h>

#include "Global.h"
#include "Database.h"
#include "Enum.h"
#include "Process.h"
#include "Simulator.h"
#include "Stack.h"
#include "ModuleTop.h"
#include "Vcd.h"

int            gConstructCount[eType_Max];
int            gDestructCount[eType_Max];
//
Database      *gDB;
SimulatorTime *gSimTime;
Simulator     *gSimulator;
Bit           *gSavedBit;
Vcd           *gVcd;

bool       gDbg = 1;
int        gRob = 1;

bool run_simulation();
bool check_objects_deleted(bool print, FILE* fp);
void show_memory_usage();

#define TEST 0
#define Assert(X,Y) if (!X) { printf("Assertion failed: %s\n", Y); if (TEST) assert(X); }

void rdbg() {
  gRob=2;
}

int main(int argc, char* argv[])
{
  // To start of, the very first thing we need is a database
  // as many classes use it to store their information.
  // We cannot create the database in run_simulation() because
  // some tables are freed only once run_simulation scope exits
  // and a 'delete gDB;' in run_simulation would be before that.
  gDB = new Database("./", "FirstVerifyRightDB");
  assert(gDB);

  run_simulation();

  delete gDB; gDB = 0;

  show_memory_usage();

  return 1;

// Change the above steps to:
//   assert( gDB = new Database("./", "FirstVerifyRightDB"));
//   gSimulator = new Simulator(...);
//   MyModule* mymodule = new MyModule(...);
//   gSimulator->Start((Module*)mymodule, time);
//   delete gSimulator;
//   delete gDB;
}

bool run_simulation()
{
  gSimTime   = new SimulatorTime; // do this inside Simulator
  gSimulator = new Simulator;
  gSavedBit  = new Bit(256);

  Module* top = (Module*)(new ModuleTop("RW_Top"));
  gVcd        = new Vcd(); // keep Vcd handle handy in case dumping is turned on.

//gVcd->DumpStart(top, "dump.vcd");
  gSimulator->SimulationTimeMax(400*1);
  gSimulator->Start(top, (Process*)0); // maybe add sim time max arg here

  delete top;
  delete gVcd;
  delete gSimulator;
  delete gSavedBit;
  delete gSimTime; // delete last so time can always be printed

  return TRUE;
}

#include "MemoryUsage.cc"

// End
