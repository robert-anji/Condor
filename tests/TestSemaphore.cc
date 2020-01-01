// $RCSfile: TestSemaphore.cc,v $
// $Revision: 1.1 $

#include <stdio.h>
#include <assert.h>

#include "Global.h"
#include "Database.h"
#include "Enum.h"
#include "List.h"
#include "Mailbox.h"
#include "Semaphore.h"
#include "Simulator.h"
#include "Vcd.h"
//
#include "Process.h"
extern Module* gModuleTop;
//
int            gConstructCount[eType_Max];
int            gDestructCount[eType_Max];
Database*      gDB;
Simulator*     gSimulator;
SimulationTime gSimTime;
Vcd*           gVcd;
bool           gDbg;
//
void show_memory_usage();
void sem_process(ProcessExecuteReason& rsn, Process* proc);

void test_semaphore();
void top_module_constr(Module* m);

int main(int argc, char* argv[])
{
  for (unsigned u=0; u<eType_Max; u++) { gConstructCount[u]=gDestructCount[0]=0; }

  test_semaphore();

  show_memory_usage();

  return 1;
}

class Trans { public: unsigned id; };

Mailbox<Trans>* gMboxPtr  = 0;
Event*          gEventPtr = 0;
Semaphore*      gSemPtr   = 0;
unsigned        gGetCount = 0;
unsigned        gPutCount = 0;

void test_semaphore()
{
  gDB = new Database("./", "FirstVerifyRightDB");

  gSimulator = new Simulator();

  Module* mod_top = new Module("Module_TestMailbox", 0, top_module_constr);
  gVcd            = new Vcd(); // keep Vcd handle handy in case dumping is turned on.

//gVcd->DumpStart(my_top, "dump.vcd");
  gSimulator->SimulationTimeMax(400*1);
  gSimulator->Start(mod_top);

  if (gMboxPtr)    delete gMboxPtr;
  if (gEventPtr)   delete gEventPtr;
  if (gSemPtr)     delete gSemPtr;

  delete mod_top;
  delete gVcd;
  delete gSimulator;
  delete gDB;
}

Trans tr;

void top_module_constr(Module* m)
{
  printf("top_module_constr\n");

  gMboxPtr = new Mailbox<Trans>("TestMbox", 10);
  //
  gSemPtr = new Semaphore("SemTest", 64, eSemaphorePolicy_Random);

  char proc_name[256];

  for (int i=0; i<64; i++) {
    sprintf(proc_name, "SemProcess%d", i);
    Process* p = new Process(proc_name, m, sem_process);
    // p->SkipInititalReason();
  }
}

void sem_process(ProcessExecuteReason& rsn, Process* proc)
{
  if (rsn.ReasonGet() == eExecuteReason_Initial) {
    printf("%s: Get\n", proc->Name());
    gSemPtr->Get(1);
    gGetCount++;
  }
  else if (rsn.ReasonGet() == eExecuteReason_SemaphoreGet) {
    if (gPutCount < 100) {
      printf("%s: Put\n", proc->Name());
      gSemPtr->Put(1);
      gPutCount++;
    }
    if (gPutCount ==100)
      printf("[%u] %s: Reached programmed Put count\n", gSimTime, proc->Name());
  }
  else if (rsn.ReasonGet() == eExecuteReason_SemaphorePut) {
    if (gGetCount < 100) {
      printf("%s: Get\n", proc->Name());
      gSemPtr->Get(1);
      gGetCount++;
    }

    if (gGetCount ==100)
      printf("[%u] %s: Reached programmed Get count\n", gSimTime, proc->Name());
  }
  else {
    assert(0); // unexpected reason
  }
}


#include "MemoryUsage.cc"

// End
