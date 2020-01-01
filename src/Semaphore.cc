// $RCSfile: Semaphore.cc,v $
// $Revision: 1.4 $

#include <assert.h>

#include "Semaphore.h"

#include "Global.h"
#include "Scheduler.h"
#include "SimulationUtilities.h"

void dbg_handler()
{
  int i;
  i = 0;
  assert(i);
}

static unsigned sLoopCount[26];
#define M_LOOP_INIT(id)      { sLoopCount[id] = 0; }
#define M_LOOP_MAX(id, max)  { if (sLoopCount[id]++ > max) dbg_handler(); }

extern Scheduler* gScheduler;
//
M_ExternConstructDestructCountInt;

Semaphore::Semaphore(const char* name, unsigned total_keys, eSemaphorePolicy policy) :
  mName          (name),
  mKeysTotal     (total_keys),
  mKeysRemaining (total_keys),
  mPolicy        (policy),
  mpPRNG         (0)
{
  M_UpdConstructCount(eType_Semaphore);
}

Semaphore::~Semaphore()
{
  if (mpPRNG) { delete mpPRNG; mpPRNG=0; }

  M_ListDeleteAll(mSemInfoGetPending,   SemaphoreInfo);

  M_UpdDestructCount(eType_Semaphore);
}

bool Semaphore::Get(unsigned num_keys)
{
  // Probably best to treat '0 key' Get request as an error.
  assert(num_keys);

  // Cannot ask for more keys than the maximum possible.
  assert(num_keys <= mKeysTotal);

  SemaphoreInfo* sem_info = mObjReuseSem.New(FALSE);
  assert(sem_info);
  //
  sem_info->mpProcess = GetCurrentProcess();
  sem_info->mNumKeys  = num_keys;
  //
  mSemInfoGetPending.Add(sem_info);

  return gScheduler->ScheduleSemaphoreGet(this);
}

bool Semaphore::Put(unsigned num_keys)
{
  SemaphoreInfo* sem_info = mObjReuseSem.New(FALSE);
  assert(sem_info);

  sem_info->mpProcess = GetCurrentProcess();
  sem_info->mNumKeys  = num_keys;
  //
  mSemInfoPutPending.Add(sem_info);

  return gScheduler->ScheduleSemaphorePut(this);
}

Process* Semaphore::FlushGets()
{
  bool     found   = FALSE;
  Process* process = 0;

  if (mPolicy==eSemaphorePolicy_RoundRobinWithBlock
  ||  mPolicy==eSemaphorePolicy_RoundRobinWithNoBlock)
  {
    for (M_ListIterateAll(mSemInfoGetPending)) {
      SemaphoreInfo* sem_info = mSemInfoGetPending.Get();
      assert(sem_info);

      process = sem_info->mpProcess;

      if (sem_info->mNumKeys <= mKeysRemaining) {
        if (gScheduler->ProcessIsBlocked(process)) {
          if (! gScheduler->ProcessUnblock(process, this, eSyncArg_Get))
            continue;
        }

        found = TRUE;

        mKeysRemaining -= sem_info->mNumKeys;

        // Move the sem_info to the allocated list.
        mSemInfoGetAllocated.Add(sem_info);

        mSemInfoGetPending.DeleteLinkThenIterationDone();
      }

      if (mPolicy == eSemaphorePolicy_RoundRobinWithBlock) {
        // If we reach here we do not look for further pending Gets, the current
        // process may be returned if it wasn't blocked and keys were available.
        mSemInfoGetPending.Last(); // force iteration to end
      }
    }
  }

  if (mPolicy == eSemaphorePolicy_Random) {
    if (! mpPRNG) mpPRNG = new PRNG(SeedGet()+mName.Hash());

    if (mSemInfoGetPending.Size()) {
      unsigned pos = mpPRNG->RandomValueGet0To(mSemInfoGetPending.Size());

      mSemInfoGetPending.PositionSet(pos);
      mSemInfoGetPending.PositionSave();

      for (int l=0; l<2 && !found; l++) {
        if (l == 1) mSemInfoGetPending.PositionRestore();

        do {
          SemaphoreInfo* sem_info = mSemInfoGetPending.Get();
          assert(sem_info);

          if (sem_info->mNumKeys <= mKeysRemaining) {
            process = sem_info->mpProcess;

            if (gScheduler->ProcessIsBlocked(process)) {
              if (! gScheduler->ProcessUnblock(process, this, eSyncArg_Get))
                continue;
            }

            found = TRUE;

            mKeysRemaining -= sem_info->mNumKeys;

            // Move the sem_info to the allocated list.
            mSemInfoGetAllocated.Add(sem_info);

            mSemInfoGetPending.DeleteLinkThenIterationDone();
            break;
          }
        } while ((l==0) ? mSemInfoGetPending.Next() : mSemInfoGetPending.Prev());
      }
    }
  }

  if (found) return process;
  else       return 0;
}

Process* Semaphore::FlushPuts()
{
  Process* process = 0;
  bool     found   = FALSE;
  unsigned keys_to_put;

  if (! mSemInfoPutPending.First()) return process;

  SemaphoreInfo* sem_info_put = mSemInfoPutPending.Get();
  assert(sem_info_put);

  // Note: Preceeding Get request is always > 0 so keys_to_put must be > 0.
  keys_to_put = sem_info_put->mNumKeys;
  process     = sem_info_put->mpProcess;

  // Make sure the Put for the process corresponds
  // to one or more Gets from the same process.
  M_LOOP_INIT(0);
  for (mSemInfoGetAllocated.First(); keys_to_put && !mSemInfoGetAllocated.IsDone(); )
  {
    SemaphoreInfo* sem_info_alloc = mSemInfoGetAllocated.Get();
    assert(sem_info_alloc);

    M_LOOP_MAX(0, 10000);

    if (sem_info_alloc->mpProcess == process) {
      found = TRUE;

      if (keys_to_put >= sem_info_alloc->mNumKeys) {

        // Allowed to return as many keys as was taken in one or more Gets().
        keys_to_put -= sem_info_alloc->mNumKeys;

        mKeysRemaining += sem_info_alloc->mNumKeys;

      //delete sem_info_alloc;
        mObjReuseSem.Free(sem_info_alloc);

        mSemInfoGetAllocated.DeleteLinkThenNext();

        continue;
      }
      else {
        sem_info_alloc->mNumKeys -= keys_to_put;
        mKeysRemaining           += keys_to_put;
        keys_to_put               = 0;

        // Leave the link intact as it has non-zero keys remaining.
      }
    }

    if (keys_to_put) mSemInfoGetAllocated.Next();
  }
 
  // Make sure the corresponding Get process was found for the Put
  // and that a legal number of keys was specified in the Put.
  assert(found);
  assert(keys_to_put == 0);

  if (found && keys_to_put==0) {
  //delete sem_info_put;
    mObjReuseSem.Free(sem_info_put);
    mSemInfoPutPending.DeleteLinkThenNext();
  }

  return process;
}

// End
