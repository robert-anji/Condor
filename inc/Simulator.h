// $RCSfile: Simulator.h,v $
// $Revision: 1.4 $

#ifndef SimulatorClass
#define SimulatorClass

#include "Global.h"
#include "SimulatorDefines.h"
#include "String.h"

//typedef unsigned SimulationTime;

#include "List.h"
#include "SimulatorTime.h"
#include "String.h"

M_ExternConstructDestructCountInt;

class Module;
class Process;
class Scheduler;

struct SensitiveInfo {
  SensitiveInfo() : mpProcess(0), mSensitiveType(eSensitiveType_None)
    { M_UpdConstructCount(eType_SensitiveInfo); }
 ~SensitiveInfo()
    { M_UpdDestructCount(eType_SensitiveInfo);  }

  Process*       mpProcess;
  String         mSignalName;
  eSensitiveType mSensitiveType;
};

class Simulator
{
public:
  Simulator(unsigned seed=13);
 ~Simulator();

  unsigned SeedGet() { return mSeed; }

  // Functions that simply pass-thru to the Scheduler:
  void SimulationTimeMax(SimulatorTime& tm);
  void SimulationTimeMax(unsigned       tm);
  bool Start(Module* module_top, Process* software_top);

  // Query functions;
  Module* ModuleTopGet() const { return mpModuleTop; }

private:
  String        mSimulatorVersion;
  unsigned      mSeed;
  Scheduler*    mpScheduler;
  Module*       mpModuleTop;
  Process*      mpSoftwareTop;

private:
  bool ElaborateModule          (Module *module_top);
  bool ElaborateModuleRecursive (Module *parent_module) const;
};


#endif

// End
