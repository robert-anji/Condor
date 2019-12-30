// $RCSfile: Module.h,v $
// $Revision: 1.5 $

#ifndef ModuleClass
#define ModuleClass

#include "Interface.h"
#include "List.h"
#include "String.h"

static unsigned sModuleCount;

class Process;
class Clock;

enum eModuleRelation {
  eModule1IsChild,
  eModule2IsChild,
  eInterModule
};

class Module;

typedef void (*ModuleConstrFunctionPtr)(Module* m);
typedef        ModuleConstrFunctionPtr  ModuleConstrFnPtr;

class Module
{
  friend class Scheduler;
  friend class Simulator;
  friend class Vcd;

public:
  Module(const char* name, Module* parent_module);
  Module(const char* name, Module* parent_module, ModuleConstrFnPtr fn_ptr);
  virtual ~Module();

  virtual bool Build() {}
  //
  bool ProcessAdd(Process* process);

  // Tasks to connect interfaces.
  bool ConnectInterfaceGroup(Interface* intf1, Interface* intf2, const char* group_name, eInterfaceConnectType type);
  bool ConnectInterfaceGroup(Interface* intf1, const char* grp_name1, Interface* intf2, const char* grp_name2, eInterfaceConnectType type);
//TBD:
//bool ConnectSignal(Interface* intf1, Interface* intf2, const char* signal_name);
//bool ConnectSignal(Interface* intf1, const char* signal_name1, Interface* intf2, const char* signal_name2);

  // Query methods.
  bool        IsChildModuleOf(Module* mod);
  Module*     ParentModule() { return mpParentModule;     }
  const char* Name()         { return (const char*)mName; }
  //
  Process*    ProcessFind(const char* process_name, bool search_in_submodules);

private:
  String        mName;
  Module*       mpParentModule;
  ModuleConstrFnPtr mModuleConstrFnPtr;
  List<Module>  mSubModulesList;
  List<Process> mProcessesList;
  unsigned      mModuleId;
  unsigned      mModuleDepth;
};

#endif

// End
