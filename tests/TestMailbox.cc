// $RCSfile: TestMailbox.cc,v $
// $Revision: 1.1 $

#include <stdio.h>

#include "Global.h"
#include "Database.h"
#include "Enum.h"
#include "List.h"
#include "Mailbox.h"
#include "SeedHelper.h"
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
void process_fn1(ProcessExecuteReason& rsn, Process* proc);
void process_fn2(ProcessExecuteReason& rsn, Process* proc);

void test_mailbox();

int main(int argc, char* argv[])
{
  for (unsigned u=0; u<eType_Max; u++) { gConstructCount[u]=gDestructCount[0]=0; }

  test_mailbox();

  show_memory_usage();

  return 1;
}

class Trans { public: unsigned id; };

Mailbox<Trans>* mbox_ptr;
Event* e1;

void test_mailbox()
{
  e1 = 0;

  gDB = new Database("./", "FirstVerifyRightDB");

  gSimulator = new Simulator();

  Module* parent_module;

  Module* mod_top = new Module("Module_TestMailbox", parent_module=0);
  gVcd            = new Vcd(); // keep Vcd handle handy in case dumping is turned on.

  mbox_ptr = new Mailbox<Trans>("test_mbox", 10);
  //
  new Process("TestProcess1", mod_top, process_fn1);
  new Process("TestProcess2", mod_top, process_fn2);

//gVcd->DumpStart(my_top, "dump.vcd");
  gSimulator->SimulationTimeMax(400*1);
  gSimulator->Start(mod_top);

  delete mbox_ptr;
  if (e1) delete e1;

  delete mod_top;
  delete gVcd;
  delete gSimulator;
  delete gDB;
}

Trans tr;

void process_fn1(ProcessExecuteReason& rsn, Process* proc)
{
  if (rsn.ReasonGet() == eExecuteReason_Initial) {
    e1 = new Event("EventTest");
    e1->Set();

    printf("Hello there!\n");

    tr.id = 5;
    mbox_ptr->Put(&tr);
  }

  if (rsn.ReasonGet() == eExecuteReason_MailboxPut) {
    printf("Done putting the transaction to mailbox\n");
  }
}

void process_fn2(ProcessExecuteReason& rsn, Process* proc)
{
  Process* p = GetCurrentProcess();

  if (rsn.ReasonGet() == eExecuteReason_Initial) {
    if (e1) e1->Get();
    if (e1) Wait(e1);

    mbox_ptr->Get();

    printf("Hows it going!\n");
  }

  if (rsn.ReasonGet() == eExecuteReason_Event) {
    printf("Got Event!\n");
  }

  if (rsn.ReasonGet() == eExecuteReason_MailboxGet) {
    printf("Getting the transaction from the mailbox\n");
    Trans* tr = (Trans*)rsn.ReasonObjGet(eExecuteReasonObj_Mailbox);

    printf("TransId=%d\n", tr->id);
  }
}

#include "MemoryUsage.cc"

// End
