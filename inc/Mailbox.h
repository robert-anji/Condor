// $RCSfile: Mailbox.h,v $
// $Revision: 1.4 $

#ifndef MailboxClass
#define MailboxClass

#define D_MailboxEntriesMax 10000

#include "MailboxBase.h"
#include "SimulatorDefines.h"
#include "String.h"

class Process;

template <class T>
class Mailbox : public MailboxBase
{
  friend class Scheduler;

public:
  Mailbox(const char* name, unsigned max_entries);
 ~Mailbox();

  // Add/Remove entries with potential blocking of the calling 'process'.
  // Cannot specify eBlockProcess type as Get() agent unblocks.
  bool Put(T* t);
  void Get();

private:
  unsigned mMailboxId;
  unsigned mEntriesMax;
};

#include "Mailbox.cc"

#endif

// End
