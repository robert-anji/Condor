// $RCSfile: MailboxBase.cc,v $
// $Revision: 1.1 $

#include <assert.h>

#include "MailboxBase.h"
//
#include "Global.h"
#include "Scheduler.h"

extern Scheduler* gScheduler;

MailboxBase::MailboxBase(const char* name) :
  mName           (name)
{
  M_UpdConstructCount(eType_MailboxBase);
}

MailboxBase::~MailboxBase()
{
  M_UpdDestructCount(eType_MailboxBase);
}

bool MailboxBase::FlushPutsAndGets(Process** proc_put, Process** proc_get, void** elm)
{
  bool schedule_mbox_put = FALSE;
  bool found_get;

  *elm = 0;

  // For each mailbox, find a 'Put' entry and a non-blocked 'Get' entry.
  // If found, schedule the 'Put' process unless it is blocked,
  // then schedule the 'Get' entry.
  // Note: The term 'blocked' means that the process is cuurently waiting
  //       exclusively on some other synchronizing agent.

  mMboxElmWaitingOnPutList.First(); // keep Elm-list in sync with Put-list

  for (M_ListIterateAll(mProcessWaitingOnPutList)) {
    // Note: the PutList entry will not get deleted if we break from the loop.

    *proc_put = mProcessWaitingOnPutList.Get();
    assert(*proc_put);

    // We have found a 'Put' entry, now see if there is also a waiting 'Get'.
    found_get = FALSE;
    //
    for (M_ListIterateAll(mProcessWaitingOnGetList)) {
      *proc_get = mProcessWaitingOnGetList.Get();
      assert(*proc_get);

      if (gScheduler->ProcessIsBlocked(*proc_get)
      && !gScheduler->ProcessUnblock(*proc_get, this, eSyncArg_Get))
      {
        continue; // this process will miss the mailbox put entry!
      }

      mProcessWaitingOnGetList.DeleteLinkThenIterationDone();

      found_get = TRUE;
    }

    if (found_get) {
      // Transfer the mailbox element from 'Put' to 'Get' and
      // check if we need to schedule the Put process then
      // delete the Put entry.
      *elm = mMboxElmWaitingOnPutList.Get();
      mMboxElmWaitingOnPutList.DeleteLinkThenNext(); // pos is actually dont-care
      //
      if (gScheduler->ProcessIsBlocked(*proc_put)
      && !gScheduler->ProcessUnblock(*proc_put, this, eSyncArg_Put))
      {
        *proc_put = 0; // zeroed out, Scheduler will not schedule proc_put
      }
      //
      mProcessWaitingOnPutList.DeleteLinkThenIterationDone();
    }
    else {
      *proc_get = 0;
      break; // no 'Get' found, break out of 'Put' loop, no Put is consumed
    }
  }

  return found_get;
}

// End
