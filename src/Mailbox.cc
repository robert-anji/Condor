// $RCSfile: Mailbox.cc,v $
// $Revision: 1.6 $

#include <assert.h>

#include "Global.h"
#include "Scheduler.h"
#include "SimulationUtilities.h"
 
extern Scheduler* gScheduler;
//
M_ExternConstructDestructCountInt;

static unsigned sMailboxCount = 0;

template <class T>
Mailbox<T>::Mailbox(const char* name, unsigned max_entries) :
  MailboxBase   (name),
  mMailboxId    (++sMailboxCount),
  mEntriesMax   (max_entries)
{
  assert(mEntriesMax <= D_MailboxEntriesMax);

//
//  ---------------
//  | g:Scheduler |   <--- MailbxAdd() ---- this;
//  ---------------
//

  gScheduler->MailboxAdd((MailboxBase*)this);

  M_UpdConstructCount(eType_Mailbox);
}

template <class T>
Mailbox<T>::~Mailbox()
{
  M_UpdDestructCount(eType_Mailbox);
}

template <class T>
bool Mailbox<T>::Put(T* t)
{
  // Note: The value of block_type (Exclusive, Inclusive) does not block the
  //       element 't' from a waiting Get, it only blocks the Put process.

  if (mProcessWaitingOnPutList.Size() == mEntriesMax) return FALSE;

  assert(mProcessWaitingOnPutList.Add(GetCurrentProcess()));
  assert(mMboxElmWaitingOnPutList.Add((void*)t));

  gScheduler->ScheduleMailbox();

  return TRUE;
}

template <class T>
void Mailbox<T>::Get()
{
  mProcessWaitingOnGetList.Add(GetCurrentProcess());

  gScheduler->ScheduleMailbox();
}

// End
