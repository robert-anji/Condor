// $RCSfile: Module.cc,v $
// $Revision: 1.5 $

#include <assert.h>

#include "Module.h"
//
#include "Clock.h"
#include "Global.h"
#include "Interface.h"
#include "Process.h"
#include "Scheduler.h"
#include "Simulator.h"

M_ExternConstructDestructCountInt;

extern Simulator* gSimulator;

Module::Module(const char* name, Module* parent_module) :
  mName              (name),
  mpParentModule     (parent_module),
  mModuleConstrFnPtr (0),
  mModuleId          (sModuleCount++),
  mModuleDepth       (0)
{
  if (mpParentModule) {
    mModuleDepth = mpParentModule->mModuleDepth + 1;

    mpParentModule->mSubModulesList.Add(this);
  }

  M_UpdConstructCount(eType_Module);
}

Module::Module(const char* name, Module* parent_module, ModuleConstrFnPtr fn_ptr) :
  mName              (name),
  mpParentModule     (parent_module),
  mModuleConstrFnPtr (fn_ptr),
  mModuleId          (sModuleCount++),
  mModuleDepth       (0)
{
  if (mpParentModule) {
    mModuleDepth = mpParentModule->mModuleDepth + 1;

    mpParentModule->mSubModulesList.Add(this);
  }

  if (fn_ptr) (*fn_ptr)(this);

  M_UpdConstructCount(eType_Module);
}

Module::~Module()
{
  MacroListDeleteAll(mProcessesList, Process);

  M_UpdDestructCount(eType_Module);
}

bool Module::ProcessAdd(Process* process)
{
  assert (process);

  // TBD: Confirm a process with the same name is not already present.
  return mProcessesList.Add(process);
}

bool Module::ConnectInterfaceGroup(Interface* intf1, Interface* intf2, const char* group_name, eInterfaceConnectType type)
{
  return ConnectInterfaceGroup(intf1, group_name, intf2, group_name, type);
}

bool Module::ConnectInterfaceGroup(Interface* intf1, const char* group_name1, Interface* intf2, const char* group_name2, eInterfaceConnectType type)
{
  MacroAssert4(intf1, group_name1, intf2, group_name2);

  bool ignore_group1 = FALSE;
  bool ignore_group2 = FALSE;

  int found_count = 0;

  if (!strlen(group_name1)) ignore_group1 = TRUE;
  if (!strlen(group_name2)) ignore_group2 = TRUE;

  bool error = FALSE;

  for (int i=0; !error; i++) {
    if (intf1->mpInterfaceDescription[i].signal_width == 0) break;
    // IMPR: Check if intf2 still has unconnected signals for group_name2, that is an error,

    const char* s1 = intf1->mpInterfaceDescription[i].signal_name;
    const char* g1 = intf1->mpInterfaceDescription[i].signal_group;
    eSignalType t1 = intf1->mpInterfaceDescription[i].signal_type;
    bool   is_clk1 =    intf1->mpOwningProcess // for clocks, intf & process are created together
                     && intf1->mpOwningProcess->ProcessType()==eProcessType_Clock;

    // IMPR: g1 and g2 can describe multiple group names (separated by .).
    //       temp solution is to use strstr() below but that doesnt work if
    //       one group name is a sub-string of another group name.
    if (!ignore_group1 && !strstr(g1, group_name1)) continue;

    for (int j=0; !error; j++) {
      const char* s2 = intf2->mpInterfaceDescription[j].signal_name;
      const char* g2 = intf2->mpInterfaceDescription[j].signal_group;
      eSignalType t2 = intf2->mpInterfaceDescription[j].signal_type;
      bool   is_clk2 =    intf2->mpOwningProcess // for clocks, intf & process are created together
                       && intf2->mpOwningProcess->ProcessType()==eProcessType_Clock;

      if (!strlen(s2)) {
        printf("Error, signal %s not found in interface %s", s1, intf2->Name()), M_FL;
        assert(0); // no matching name (signal) found
        break;
      }

      if (!ignore_group2 && !strstr(g2, group_name2)) continue;

      // Signal names must match except for clock processes.
      // For clock processes the groups matching is sufficient.
      if (strcmp(s1, s2) && !is_clk1 && !is_clk2) continue;

      Signal* signal1 = intf1->SignalGet(s1);
      Signal* signal2 = intf2->SignalGet(s2);
      MacroAssert2(signal1, signal2);

      // When we create an interface with signal directions reversed the clock
      // input also gets reversed and becomes an output, we fix this here
      // (behind the scenes) because we can now assume the signal is meant to
      // be a clock input.
      if (is_clk1) signal2->MarkAsInputIfReversed();
      if (is_clk2) signal1->MarkAsInputIfReversed();

      // Either intf1 or intf2 can be used to connect the signals.
      intf1->Connect(signal1, signal2, type, error);

      if (error) {
        printf("Error: Not able to connect %s with %s", s1, s2), M_FL;
        assert(0);
      }
      else {
        if (is_clk1) intf2->SetAsSynchronous();
        if (is_clk2) intf1->SetAsSynchronous();

        found_count++;
        break;
      }
    }
  }
  
  return (error==FALSE && found_count>0);
}

Process* Module::ProcessFind(const char* process_name, bool search_in_submodules)
{
  bool     found   = FALSE;
  Process* process = 0;

  // Try to find the process in the current module, then look in sub-modules.
  for (MacroListIterateAll(mProcessesList)) {
    process = mProcessesList.Get();
    assert(process);

    if (!strcmp(process->Name(), process_name)) { found=TRUE; break; }
  }

  if      (found)                 return process;
  else if (!search_in_submodules) return 0;
  // else (continue below)

  for (M_ListIterateAll(mSubModulesList)) {
    Module* module = mSubModulesList.Get();
    assert(module);

    if (process = module->ProcessFind(process_name, search_in_submodules=TRUE))
      return process;
  }

  return process;
}

bool Module::IsChildModuleOf(Module* mod)
{
  assert(mod);

  Module* mod_parent = this->mpParentModule; 

  while (mod_parent) {
    if (mod_parent == mod) return TRUE;
    else                   mod_parent = mod_parent->mpParentModule;
  }

  return FALSE;
}

// End
