// $RCSfile: Semaphore.h,v $
// $Revision: 1.2 $

#ifndef SemaphoreClass
#define SemaphoreClass

#include "List.h"
#include "ObjectReuse.h"
#include "PRNG.h"
#include "SimulatorDefines.h"
#include "String.h"

class Process;

struct SemaphoreInfo {
  SemaphoreInfo() { M_UpdConstructCount(eType_SemaphoreInfo); }
 ~SemaphoreInfo() { M_UpdDestructCount (eType_SemaphoreInfo); }

  void Clear() { mpProcess=(Process*)0; mNumKeys=0; }

  Process* mpProcess;
  unsigned mNumKeys;
};

enum eSemaphorePolicy {
  eSemaphorePolicy_RoundRobinWithBlock,   // subsequent Get requests blocked
                                          //   until current is satisfied
  eSemaphorePolicy_RoundRobinWithNoBlock, // subsequent Get request are tried
                                          //   if current can't be satisfied
  eSemaphorePolicy_Random                 // randomly pick a waiting Get
};

class Semaphore
{
friend class Scheduler;

public:
  Semaphore(const char* name, unsigned total_keys, eSemaphorePolicy policy=eSemaphorePolicy_RoundRobinWithBlock);
 ~Semaphore();

  bool Get(unsigned num_keys);
  bool Put(unsigned num_keys);

  const char* Name() { return mName; }

private:
  String           mName;
  unsigned         mKeysTotal;
  unsigned         mKeysRemaining;
  eSemaphorePolicy mPolicy;
  PRNG*            mpPRNG;

  List<SemaphoreInfo> mSemInfoGetPending;
  List<SemaphoreInfo> mSemInfoPutPending;
  List<SemaphoreInfo> mSemInfoGetAllocated;
  ObjectReuse<SemaphoreInfo> mObjReuseSem;

  Process* FlushGets(); // for Scheduler only
  Process* FlushPuts(); //         "
};

#endif
