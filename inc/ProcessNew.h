//friend class Module;
//friend class Scheduler;
//friend class Vcd;

static int sProcessId = 100;

class ProcessBase
{
public:
  ProcessBase(const char* name);

  virtual ~ProcessBase();

  virtual eProcessType  ProcessType() const; // make pure virtual?

//int ProcessId();

  // Query functions.
  // Maybe, as far as possible, always use database to query attributes.
  // We can query for: Name, State, Parent, Children etc.
//const char* Name() const { return (const char*)mProcessName; }

protected:
  String           mProcessName;
  eProcessState    mProcessState;
  ProcessExecFnPtr mpProcessExecFnPtr;

  virtual void          ProcessExecute(ProcessExecuteReason& exec_reason);
  virtual bool          ProcessStateModify(eProcessState state);
  virtual eProcessState ProcessStateGet() { return mProcessState; }

  void*   SchedulerProcessInfo; // void*    mpBlockingAgentPending;
                                // unsigned mBlockAgentArgPending;
                                // Record*  mpRecord;
                                // unsigned mDeltaCycle; // process was last executed
                                // bool     mUnblockedInCurrentDeltaCycle;
                                // bool     mHasExecutedInitial;
};

class ProcessHardware : public virtual ProcessBase
{
public:
  ProcessHardware(const char* name, Module* parent_module=0, Interface* intf=0);
  virtual ~ProcessHardware();

  // Eventually use database for all attribute queries.
  Interface* InterfaceGet() { return mpInterface; }
  bool       InterfaceAdd(Interface* intf); // only one invocation allowed initially

  bool       IsSensitiveTo(const char* signal_name, eSensitiveType edge);
  bool       IsSensitiveTo(SimulatorEvent* evt); // could be timer, sem, mbox, event etc.

  virtual eProcessType  ProcessType() const;

  Module*     GetOwningModule() const { return mpModule;                  }

protected:
  Interface*      mpInterface;
  Module*         mpModule;
  //
  List<SensitiveInfo> mSensitiveInfoList;

private:
  virtual void ProcessExecute(ProcessExecuteReason& exec_reason);
};

// I dont think this process is needed?
class ProcessInitial : public virtual ProcessBase
{
  ProcessInitial(const char* name);
  virtual ~ProcessInitial();
};

class ProcessSoftware : public virtual ProcessBase
{
public:
  ProcessSoftware(const char* name, ProcessSoftware* parent_process);
  virtual ~ProcessSoftware();

  void Wait(Timer* timer); // Non-clocked timer
  void Wait(Event* event);
  void Wait(Semaphore*   sem,  eSyncArg sync_arg);
  void Wait(MailboxBase* mbox, eSyncArg sync_arg);
  void Wait(unsigned tm, eTimeScale ts);
  //              OR
  void Wait(Event e); // where Event also supports: SimulationTime, Timer,
                      // Semaphore, Mailbox, InputChange, OutputChange etc.

protected:
  ProcessSoftware*           mpProcessParent;
  LinkList<ProcessSoftware*> mProcessChildrenList

private:
  virtual void ProcessExecute(ProcessExecuteReason& exec_reason);
};

class ProcessSoftwareHardware : public ProcessSoftware, ProcessHardware
{
};
#define ProcessBFM ProcessSoftwareHardware

// End
