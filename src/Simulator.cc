// $RCSfile: Simulator.cc,v $
// $Revision: 1.6 $

#include <assert.h>

#include "Simulator.h"
//
#include "Global.h"
#include "Module.h"
#include "Scheduler.h"

Scheduler* gScheduler;
Module*    gModuleTop;
Process*   gSoftwareTop;

NameVal SignalEdgeNV[] =
{
  eSensitiveType_None,                    "EdgeNone",
  eSensitiveType_PositiveEdge,            "PositiveEdge",
  eSensitiveType_PositiveLevel,           "PositiveLevel",
  eSensitiveType_NegativeEdge,            "NegativeEdge",
  eSensitiveType_NegativeLevel,           "NegativeLevel",
  eSensitiveType_PositiveAndNegativeEdge, "PositiveAndNegativeEdge",
  eSensitiveType_Invalid,                 "EdgeInvalid"
};

Simulator::Simulator(unsigned seed) :
  mSimulatorVersion     ("0.1"),
  mSeed                 (seed),
  mpScheduler           (0),
  mpModuleTop           (0),
  mpSoftwareTop         (0)
{
  M_UpdConstructCount(eType_Simulator);

  mpScheduler = new Scheduler();
  assert(mpScheduler);

  gScheduler = mpScheduler; // make available globally
}

Simulator::~Simulator()
{
  delete mpScheduler; mpScheduler = 0;

  M_UpdDestructCount(eType_Simulator);
}

void Simulator::SimulationTimeMax(SimulatorTime& tm)
{
  gScheduler->SimulationTimeMax(tm);
}

void Simulator::SimulationTimeMax(unsigned tm)
{
  gScheduler->SimulationTimeMax(tm);
}

bool Simulator::Start(Module* module_top, Process* software_top)
{
  mpModuleTop   = gModuleTop   = module_top;
  mpSoftwareTop = gSoftwareTop = software_top;

  assert(ElaborateModule(mpModuleTop));

  return gScheduler->Start();
}

bool Simulator::ElaborateModule(Module* module)
{
  return ElaborateModuleRecursive(mpModuleTop);
}

// The 'Elaborate' step will wrap up any unfinished business needed before
// actual simulation can start:
// - schedule all proceeses within modules
// - do a post-check on interface connections (legality or output-ouput
//   connections etc.
bool Simulator::ElaborateModuleRecursive(Module* parent_module) const
{
  // This function adds all processes within the module to the Scheduler.
  assert (parent_module);

  // First, 'elaborate' any child modules within this module.
  for (M_ListIterateAll(parent_module->mSubModulesList)) {
    Module* sub_module = (Module*)(parent_module->mSubModulesList.Get());
    assert (sub_module);

    return ElaborateModuleRecursive(sub_module);
  }

  bool result = TRUE;

  // Second, elaborate: add all processes within this module.
  for (M_ListIterateAll(parent_module->mProcessesList)) {
    Process* process = parent_module->mProcessesList.Get();
    assert(process);

    printf("Adding process %s to Scheduler", process->Name()), M_FL;
    result = result & gScheduler->ProcessSchedule(process);
  }

  return result;
}

// End
