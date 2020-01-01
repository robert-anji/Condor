// $RCSfile: Scheduler.h,v $
// $Revision: 1.7 $

// vim:foldmarker={,}

#ifndef SchedulerClass
#define SchedulerClass

#include <stdio.h>

#include "DBTables.h"
#include "Event.h"
#include "List.h"
#include "MailboxBase.h"
#include "Module.h"
#include "Process.h"
#include "Query.h"
#include "SimulatorDefines.h"
#include "Simulator.h"
#include "SimulatorTime.h"
#include "Value.h"

// Forward declarations.
class Attribute;
class Clock;
class Database;
class Process;
class Record;
class Semaphore;
class Table;
class Timer;
class Trigger;
class View;
class ViewIterator;

enum eDeltaCycle {
  eDeltaCycle_NotSet,
  eDeltaCycle_First
};

enum eSchedulerState {
  eSchedulerState_NotStarted,
  eSchedulerState_Started,
  eSchedulerState_SimTimeMaxReached,
  eSchedulerState_Finished
};

enum eSchedulerRegion {
  eSchedulerRegion_0,
  eSchedulerRegion_1,
  eSchedulerRegion_2,
  eSchedulerRegion_3,
  eSchedulerRegion_4,
  eSchedulerRegion_5,
  eSchedulerRegion_6,
  eSchedulerRegion_7,
  eSchedulerRegion_8,
  eSchedulerRegion_9,
  eSchedulerRegion_10,
  //      Useful eSchedulerRegion aliases.
  eSchedulerRegion_Async   = eSchedulerRegion_10,
  eSchedulerRegion_First   = eSchedulerRegion_0,
  eSchedulerRegion_Last    = eSchedulerRegion_10,
  eSchedulerRegion_Invalid = eSchedulerRegion_Last+1
};

enum eSchedulerDeltaCycle {
  eSchedulerDeltaCycle_Max = 1024
};

struct sProcessAndEdge {
  sProcessAndEdge() { M_UpdConstructCount(eType_ProcessAndEdgeInfo); }
 ~sProcessAndEdge() { M_UpdDestructCount (eType_ProcessAndEdgeInfo); }

  Process*       mpProcess;
  eSensitiveType mEdge;
};
typedef sProcessAndEdge ProcessAndEdgeInfo;

class Scheduler
{
public:
  Scheduler();
 ~Scheduler();

  // The Start() function will begin invoking all the registered Processes whose
  // clocks are active. This is the main purpose of the Scheduler.
  bool Start();

  // Set maximum simulation time (watchdog).
  void SimulationTimeMax(SimulatorTime& tm);
  void SimulationTimeMax(unsigned       tm);
  eTimeScale SchedulerTimeScaleGet() { return mTimeScale; }

  // Schedule all processes created within a module (includes sub-modules).
  bool ScheduleEvent(Event* e)         { if (!mEventSetList.Exists(e))  mEventSetList.Add(e); }
  void ScheduleMailbox()               { mMailboxEvaluate = TRUE; }
  bool ScheduleSemaphoreGet(Semaphore* s) { if (!mSemaphoreGetList.Exists(s)) mSemaphoreGetList.Add(s); }
  bool ScheduleSemaphorePut(Semaphore* s) { if (!mSemaphorePutList.Exists(s)) mSemaphorePutList.Add(s); }
  bool ScheduleTimer(Timer* timer);

  bool ProcessSchedule  (Process* p);
  bool ProcessRemove    (Process* p);
  void ProcessBlock     (Process* p, void* sync_obj=0, eSyncArg sync_arg=eSyncArg_None);
  bool ProcessUnblock   (Process* p, void* sync_obj,   eSyncArg sync_arg);
  bool ProcessIsBlocked (Process* p);
  //
  Process* GetCurrentProcess() { return mpProcessCurrent; }
  //
  bool TimerDone     (Timer*   t);

  //
  // Mailboxes that Scheduler has to manage.
  void MailboxAdd(MailboxBase* mbox_base);

  // Triggers will be added by the Database class for
  // the Scheduler to execute at the appropriate time.
  bool TriggerSchedule(Trigger* trg);

  // Query functions.
  SimulatorTime& SimulationTimeGet()    { return mSimulationTime; }
  const char*    SimulationTimeStrGet() { return (const char*)mSimulationTimeStr; }
  void           SimulationTimeSet(SimulatorTime& tms);

  // For debug only:
  void Print();
  void p();

private:
  Table*             mpProcessesTable;
  View*              mpProcessesView;
  ViewIterator*      mpProcessesViewIterSearch;  // for temporary use
  ViewIterator*      mpProcessesViewIterSearch2; // for temporary use
  //
  Process*           mpProcessCurrent;

  SimulatorTime      mSimulationTime;
  String             mSimulationTimeStr;
  SimulatorTime      mScheduleTime;
  SimulatorTime      mSimulationTimeMax;
  eSchedulerState    mSchedulerState;
  eTimeScale         mTimeScale;

  bool               mUseInternalClock;

  
  List<Process>            mSensitiveProcessList;
  List<ProcessAndEdgeInfo> mProcessAndEdgeList;
  List<MailboxBase>        mMboxList;
  List<Event>              mEventSetList;

  bool               mMailboxEvaluate;

  // Scheduler statistics:
  unsigned           mScheduleEntriesProcessed;
  bool               mIsFirstSyncProcessExecuted;
  unsigned           mDeltaCycle;

  // Store Triggers for execution.
  List<Trigger>      mTriggerList;

  // Store all synchronization objects created.
  List<Semaphore>    mSemaphoreGetList;
  List<Semaphore>    mSemaphorePutList;

  // Store all synchronization objects created.
  List<Timer>        mTimerList;

  // Share the current scheduled entry (record) between functions.
  Record*            mpCurrentSchedulerRecord;
  eSchedulerRegion   mCurrentRegion;

  List<Process>      mProcessesToBlockList;

  // Rather than retrieving attributes from a view each time, keep them ready.
  // Attribute names for table: Schedule
  const Attribute* mpAttrSimTime;      // next simulation time for the process
  const Attribute* mpAttrDeltaCycle;   // current delta-cycle (non-sync processes)
  const Attribute* mpAttrRegion;       // scheduling region
  const Attribute* mpAttrProcName;     // process name to be scheduled
  const Attribute* mpAttrClock;        // clock in case of a synchronous process
  const Attribute* mpAttrClockEdge;    // clock edge the prcess is sensitive to
  const Attribute* mpAttrProcess;      // process handle
  const Attribute* mpAttrExecReason;   // reason the process was scheduled
  const Attribute* mpAttrInterface;    // interface of the process
  const Attribute* mpAttrSyncObj;      // is process blocked on a Sync obj?
  const Attribute* mpAttrSyncArg;      // optional Sync obj arg (Put, Get)
  //
  // Attribute names for table: AsyncProcesses
  const Attribute* mpAttrAsyncProcess;
  const Attribute* mpAttrAsyncInterface;

  Query mSentinelQuery;
      
  static const short cAsyncProcReadyLoopCountMax = 5000; // catch runaway loop

  // Private enums:
  enum eScheduleRecord {
    eScheduleRecord_None,
    eScheduleRecord_New,
    eScheduleRecord_Current,
    eScheduleRecord_FromIter,
    eScheduleRecord_Existing
  };

  // Private member functions:
  // -------------------------
  // Add a process to Scheduler (note: process has max 1 interface & 1 clock).
  bool ProcessScheduleAsync(Process* p);
  bool ProcessScheduleSync (Process* p);
  bool ProcessScheduleClock(Clock*   clk);
  bool ProcessScheduleTimer(Timer*   t);
  bool ProcessScheduleSoftware(Process* s);
  bool ProcessScheduleCommon(Process* p, Clock* clk, eSensitiveType edge);
  //
  void AddSentinelRecToProcessView(); // for convenience we keep a dummy record
  //
  eSchedulerRegion RegionGet(Process* process, eSensitiveType clk_edge);
  eSchedulerRegion RegionGet(eProcessType process_type, eSensitiveType clk_edge);
  //
  // Flush (schedule) asynchronous processes that have inputs updated.
  unsigned FlushAsyncProcesses();
  // Flush (propagate) synchronization objects: Semaphore, Mailboxes and Events.
  unsigned FlushSemaphoreGets();
  unsigned FlushSemaphorePuts();
  unsigned FlushMailboxPuts();
  unsigned FlushEventsSet();
  unsigned FlushTimers();
  bool     FlushProcessesToBlock();

  // Combine the next two functions soon.
  void ScheduleProcessImpl(eScheduleRecord sched_type, Process* proc, SimulatorTime& sched_sim_time);
  void ScheduleTheEntry(eScheduleRecord sched_type, SimulatorTime& sched_sim_tm, unsigned delta_cycle, eSchedulerRegion reg, ViewIterator* iter_for_relink=0, Record* r=0);
  void             AdjustTimeScale(eTimeScale timescale);

  // Convenience members (avoid repeated calls to Constructor/Destructor).
  Value iValSimTime;

  // Convenience members (avoids repeated Constructor/Destructor calls).
  Query     *impQueryTmGtEq;
  Query     *impQueryTmGtRgGt;
  Query      impQueryProcFind;
  //
  ProcessExecuteReason  impProcExecReason;

  FILE* mSchedLogFp;
};

#endif

// End
