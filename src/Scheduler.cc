// $RCSfile: Scheduler.cc,v $
// $Revision: 1.9 $

#include <assert.h>
#include <unistd.h>

#include "Scheduler.h"
//
#include "Clock.h"
#include "Database.h"
#include "Global.h"
#include "Interface.h"
#include "Module.h"
#include "Process.h"
#include "Query.h"
#include "Record.h"
#include "Semaphore.h"
#include "SimulatorDefines.h"
#include "SimulatorTime.h"
#include "Stack.h"
#include "Table.h"
#include "Timer.h"
#include "Trigger.h"
#include "Value.h"
#include "Vcd.h"
#include "View.h"

extern Scheduler*     gScheduler;
extern Database*      gDB;
extern int            gDbg;
extern SimulatorTime* gSimTime;
extern Vcd*           gVcd;
static bool           sDbg = 1;

M_ExternConstructDestructCountInt;

extern TableAttributes cProcessesTableAttr;

static bool sLocalDbg = 0;
static unsigned sMaxReasons = 50;
static unsigned sLoopCount[26];

Scheduler::Scheduler() :
  mSimulationTime             (eTimeScale_ns),
  mSchedulerState             (eSchedulerState_NotStarted),
  mSimulationTimeStr          ("0"),
  mpProcessesTable            (0),
  mpProcessesView             (0),
  mUseInternalClock           (TRUE),
  mScheduleEntriesProcessed   (0),
  mDeltaCycle                 (0),
  mIsFirstSyncProcessExecuted (FALSE),
  mpCurrentSchedulerRecord    (0),
  mCurrentRegion              (eSchedulerRegion_Invalid),
  mMailboxEvaluate            (FALSE),
  mpProcessCurrent            (0),
  mSchedLogFp                 (0),
  impQueryTmGtEq              (0),
  impQueryTmGtRgGt            (0)
{
  mSimulationTimeMax = UINT_MAX;

  mSchedLogFp = fopen(".scheduler.log", "w");

  int tbl_open_flags =  eTableOpenFlags_PermAll;

  // Table for scheduler processes (clocks, synchronous, software, timers).
  assert(gDB);
  mpProcessesTable = gDB->TableOpen("ProcessTable", tbl_open_flags);
  assert(mpProcessesTable);
  //
  mpProcessesTable->AttributesAdd(&cProcessesTableAttr);
  mpProcessesView = mpProcessesTable->ViewCreate("SyncProcAndSoftProc", eRecPerm_OwnerAll|eRecPermBit_GroupAdd);
  assert(mpProcessesView);
  assert(mpProcessesTable->ViewEnable(mpProcessesView, TRUE));
  //
  mpAttrSimTime       = mpProcessesTable->AttributeFind(cAttr_Processes_SimTime);
  mpAttrDeltaCycle    = mpProcessesTable->AttributeFind(cAttr_Processes_DeltaCycle);
  mpAttrRegion        = mpProcessesTable->AttributeFind(cAttr_Processes_Region);
  mpAttrProcName      = mpProcessesTable->AttributeFind(cAttr_Processes_Name);
  mpAttrClock         = mpProcessesTable->AttributeFind(cAttr_Processes_Clock);
  mpAttrClockEdge     = mpProcessesTable->AttributeFind(cAttr_Processes_ClockEdge);
  mpAttrProcess       = mpProcessesTable->AttributeFind(cAttr_Processes_ProcessPtr);
  mpAttrExecReason    = mpProcessesTable->AttributeFind(cAttr_Processes_ExecReasonPtr);
  mpAttrInterface     = mpProcessesTable->AttributeFind(cAttr_Processes_InterfacePtr);
  mpAttrSyncObj       = mpProcessesTable->AttributeFind(cAttr_Processes_BlockingAgent);
  mpAttrSyncArg       = mpProcessesTable->AttributeFind(cAttr_Processes_BlockingAgentArg);
  MacroAssert5(mpAttrSimTime,  mpAttrClock,  mpAttrClockEdge,  mpAttrProcess, mpAttrInterface)
  MacroAssert5(mpAttrProcName, mpAttrRegion, mpAttrExecReason, mpAttrSyncObj, mpAttrSyncArg)
  // Note: Need mpAttrClockEdge until we have reference records implemented as an attribute.

  if (!gScheduler) gScheduler = this;
  else             assert(0); // only one Scheduler instance allowed

  // Allocate some iterators that we will need.
  mpProcessesViewIterSearch   = mpProcessesView->ViewIteratorNew();
  mpProcessesViewIterSearch2  = mpProcessesView->ViewIteratorNew();
  //
  MacroAssert2(mpProcessesViewIterSearch, mpProcessesViewIterSearch2);

  M_UpdConstructCount(eType_Scheduler);
}

Scheduler::~Scheduler()
{
  if (mpProcessesView->RecordSearchFirst()) {
    Record* rec;
    Stack<ProcessExecuteReason>* rsn_stack;

    do {
      rec = mpProcessesView->RecordCurrentGet();
      assert(rec);
      rsn_stack = (Stack<ProcessExecuteReason>*)(rec->GetValueAsPointer(mpAttrExecReason));

      if (rsn_stack) delete rsn_stack;
    } while (mpProcessesView->RecordSearchNext());
  }

  if (mpProcessesViewIterSearch) {
    assert(mpProcessesView->ViewIteratorFree(mpProcessesViewIterSearch));
    mpProcessesViewIterSearch = 0;
  }

  if (mpProcessesViewIterSearch2) {
    assert(mpProcessesView->ViewIteratorFree(mpProcessesViewIterSearch2));
    mpProcessesViewIterSearch2 = 0;
  }

  if (mpProcessesTable) {
    if (mpProcessesView) {
      mpProcessesTable->ViewDelete(mpProcessesView); mpProcessesView = 0;
    }

    gDB->TableClose(mpProcessesTable); mpProcessesTable = 0;
  }

  if (impQueryTmGtEq)   { delete impQueryTmGtEq;   impQueryTmGtEq   = 0; }
  if (impQueryTmGtRgGt) { delete impQueryTmGtRgGt; impQueryTmGtRgGt = 0; }

  M_ListDeleteAll(mProcessAndEdgeList, ProcessAndEdgeInfo);

  fclose(mSchedLogFp);

  M_UpdDestructCount(eType_Scheduler);
}

bool Scheduler::ProcessSchedule(Process* proc)
{
  assert (proc);

  switch (proc->ProcessType()) {
    case eProcessType_Asynchronous: return ProcessScheduleAsync   (        proc);
    case eProcessType_Synchronous:  return ProcessScheduleSync    (        proc);
    case eProcessType_Clock:        return ProcessScheduleClock   ((Clock*)proc);
    case eProcessType_Timer:        return ProcessScheduleTimer   ((Timer*)proc);
    case eProcessType_Software:     return ProcessScheduleSoftware(        proc);
    default:                        assert(0);
  }

  return FALSE;
}

bool Scheduler::ProcessScheduleAsync(Process* process)
{
  assert(ProcessScheduleCommon(process, (Clock*)0, eSensitiveType_Invalid));

  return TRUE;
}

bool Scheduler::ProcessScheduleClock(Clock* clk)
{
  if (clk->mTimeScale > mSimulationTime.TimeScaleGet())
    AdjustTimeScale(clk->mTimeScale);

  return ProcessScheduleCommon(clk, clk, eSensitiveType_PositiveAndNegativeEdge);
}

bool Scheduler::ProcessScheduleTimer(Timer* timer)
{
  // Simply call ProcessScheduleCommon with intf=0 and clk=0.
  return ProcessScheduleCommon((Process*)timer, 0, eSensitiveType_None);
}

bool Scheduler::ProcessScheduleSoftware(Process* process)
{
  // IMPR: Dont add software processes to Sync view, use separate structure.
  // Simply call ProcessScheduleCommon with intf=0 and clk=0.
  return ProcessScheduleCommon(process, 0, eSensitiveType_Invalid);
}

bool Scheduler::ProcessScheduleSync(Process* process)
{
  Interface* intf = process->InterfaceGet();
  assert(intf);

  // By this time each Sync process has its interface signals connected, now
  // to correctly schedule the synchronous process we need to know the clock
  // that is connected to each sensitive signal edge. The clock is known in the
  // interface info while the sensitive edge is known in the process info
  // because we separately used the following approach within a module:
  // > ConnectClock(intf, signal_name, clock);  // updates interface info
  // > ProcessIsSensitiveTo(signal_name, edge); // updates process   info

  bool found_clk = FALSE;

  for (M_ListIterateAll(process->mSensitiveInfoList)) {
    SensitiveInfo* sensitive_info = process->mSensitiveInfoList.Get();
    assert(sensitive_info);

    if (sensitive_info->mSensitiveType == eSensitiveType_None) continue;

    Signal* signal = process->mpInterface->SignalGet(sensitive_info->mSignalName);
    assert(signal);

    // Find out if the signal is being driven by a clock.
    signal->mConnectedSignalsList.First();
    Signal*  signal_out         = signal->mConnectedSignalsList.Get();
    Process* signal_out_process = signal_out->mpInterface->mpOwningProcess;

    if (signal_out_process->ProcessType() == eProcessType_Clock) {
      found_clk = TRUE;

      Clock* clk = (Clock*)signal_out_process;

      signal->IsDrivenByClock(TRUE);

      assert(ProcessScheduleCommon(process, clk, sensitive_info->mSensitiveType));

      clk->mConnectedProcessList.Add(process);
    }
    // else: We permit sensitive inputs on non-clock signals also.
  }

  if (! found_clk) {
    printf("Error: could not find any connected Clock for synchronous process %s\n", process->Name());

    return FALSE;
  }

  // TBD: Check if there are any unconnected interface clocks:

  return TRUE;
}

// Add Process/Interface/ClockEdge triple to the Scheduler.
// This mechanism of adding a process also allows a single process
// to be shared among multiple identical interface instances. In this case
// the process state should be stored in the interface object.
// NOTE: a Process can have either one or no clock and maximum one Interface.
// NOTE: a Process can be added at any time.
bool Scheduler::ProcessScheduleCommon(Process* process, Clock* clk, eSensitiveType clk_edge)
{
  // TBD: confirm this process and interface are not already scheduled.

  SimulatorTime        sched_sim_time;
  ProcessExecuteReason per;
  eProcessType         proc_type = process->ProcessType();
  //
  Stack<ProcessExecuteReason>* reason = 0;

  Interface* intf = process->InterfaceGet();

  mScheduleTime = mSimulationTime;

  switch (proc_type) {
    case eProcessType_Timer:
      reason         = new Stack<ProcessExecuteReason>(sMaxReasons);
      //
      mScheduleTime += ((Timer*)process)->TimeoutValue();
      break;
    case eProcessType_Software:
      mScheduleTime = mSimulationTime;
      reason         = new Stack<ProcessExecuteReason>(sMaxReasons);
      //
      per.mExecuteReason = eExecuteReason_Initial;
      per.mpReasonObject = 0;
      //
      reason->Push(per); // makes a copy
      fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
       process->Name(), Enum2Str(ExecuteReasonNV, per.mExecuteReason));
      break;
    case eProcessType_Clock:
      clk->Start();
      break;
    case eProcessType_Synchronous:
      assert(clk); // should be connected at this time
      reason = new Stack<ProcessExecuteReason>(sMaxReasons);
      break;
    case eProcessType_Asynchronous:
      reason         = new Stack<ProcessExecuteReason>(sMaxReasons);
      //
      per.mExecuteReason = eExecuteReason_Initial;
      per.mpReasonObject = 0;
      //
      // TBD: Mark all interface inputs as sensitive
      reason->Push(per); // makes a copy
      break;
    default: assert(0);
  }

  unsigned delta_cycle;
  //
  if (mSchedulerState == eSchedulerState_NotStarted) delta_cycle = 0;
  else                                               delta_cycle = mDeltaCycle + 1;
  //
  eSchedulerRegion region = RegionGet(process, clk_edge);
  //
  bool     auto_commit;
  Record* rec_new = mpProcessesView->RecordNew(auto_commit=TRUE);

//rec_new->mpView = mpProcessesView;
  assert(rec_new);

  process->mpRecord = rec_new;

  Record& rec = *rec_new;

  rec[mpAttrSimTime]       = mScheduleTime.TimeGet();
  rec[mpAttrDeltaCycle]    = delta_cycle;
  rec[mpAttrRegion]        = region;
  rec[mpAttrProcName]      = process->Name();
  rec[mpAttrClock]         = (void*)clk;
  rec[mpAttrClockEdge]     = clk_edge;
  rec[mpAttrProcess]       = (void*)process;
  rec[mpAttrExecReason]    = (void*)reason;
  rec[mpAttrInterface]     = (void*)intf;
  rec[mpAttrSyncObj]       = (void*)0;
  rec[mpAttrSyncArg]       = (unsigned)DONT_CARE;

  rec.Print(10, stdout);

  ScheduleProcessImpl(eScheduleRecord_New, process, mScheduleTime);

  return TRUE;
}

bool Scheduler::ProcessRemove(Process* p)
{
  Query q;

  q = (*mpAttrProcess == (void*)p);

  Record* rec_q = mpProcessesViewIterSearch->RecordSearchFirst(&q);

  if (rec_q) {
    mpProcessesView->TranRecordDelete(rec_q);
    return TRUE;
  }

  return FALSE;
}

bool Scheduler::TimerDone(Timer* timer)
{
  return mTimerList.Add(timer);
}

// Used to schedule any sensitive process.
//bool Scheduler::ScheduleSensitiveProcess(Process* process)
//{
//  ScheduleProcessImpl(eScheduleRecord_Existing, process, mScheduleTime);
//
//  return TRUE;
//}

bool Scheduler::ScheduleTimer(Timer* timer)
{
  Stack<ProcessExecuteReason>* rsn_stack;

  Record* rec = timer->mpRecord;
  assert(rec);

  rsn_stack = (Stack<ProcessExecuteReason>*)(rec->GetValueAsPointer(mpAttrExecReason));
  assert(rsn_stack);

  impProcExecReason.mExecuteReason = eExecuteReason_TimerDone;
  impProcExecReason.mpReasonObject = (void*)timer;
  rsn_stack->Push(impProcExecReason); // makes a copy
  fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
   timer->Name(), Enum2Str(ExecuteReasonNV, impProcExecReason.mExecuteReason));

  mScheduleTime  = mSimulationTime;
  mScheduleTime += timer->TimeoutValue(mSimulationTime.TimeScaleGet());

  ScheduleProcessImpl(eScheduleRecord_Existing, (Process*)timer, mScheduleTime);

  return TRUE;
}

bool Scheduler::Start()
{
  bool all_ok = TRUE;

  // There is a special case for the execution order of Synchronous Interfaces
  // at timestamp 0 due to the setting of eClockPhase for each process.
  Record*        r;
  Interface*     intf;
  Process*       proc;
  eProcessType   proc_type;
  SimulatorTime  next_process_sim_time;
  eSensitiveType clk_edge;
  Clock*         clk;
  Interface*     intf_to_commit[100];
  short          intf_count;
  short          consider_count=0;
  //
  ProcessExecuteReason proc_exec_reason;
  //
  unsigned       delta_events       = 0; // track this just for statistics
  unsigned       delta_events_total = 0; //               "

  mpProcessesView->RecordSearchFirst();

  // Algorithm steps from notes.txt: Scheduler_logic_overall_with_interfaces.
  //
  // Step1: start scheduler (commit all tables and set simulation time to 0).
  mSchedulerState = eSchedulerState_Started;
  mCurrentRegion  = eSchedulerRegion_First;
  gDB->DBTransactionsCommitAll();
  mSimulationTime = 0;

  // Step2: sync-i/f are set to default by constructor/owning process and commited.
  // Step3: async-i/f are set to default by constructor/owning process or strapped.
  // Note:  Step2 and Step3 are accomplished outside the Scheduler.

  // SchedulerMainLoop:
  while (mSchedulerState == eSchedulerState_Started)
  {
    bool    redo       = TRUE; // need at least one pass in the loop
    Record* rr;

    mDeltaCycle = 0;

//  // Step4: find & update any ready async-i/f, propagate outputs to connected inputs.
//  while (redo) {
//    redo = FALSE;

//    // IMPR: Use a dedicated list of async processes that have inputs modified
//    //       as updated during the signal commit phase instead of looking in all.
//    for (r=async_view->RecordSearchFirst(); r; r=async_view->RecordSearchNext())
//    {
//      proc = (Process*  )r->GetValueAsPointer(mpAttrProcess);
//      intf = (Interface*)r->GetValueAsPointer(mpAttrInterface);
//      assert(proc);
//      assert(intf);

//      mpProcessCurrent = proc;

//      if (intf->mInputsUpdated
//      || !proc->mHasExecutedInitial)
//      {
//        if (! proc->mHasExecutedInitial)
//          proc_exec_reason.mExecuteReason = eExecuteReason_Initial;
//        else
//          proc_exec_reason.mExecuteReason = eExecuteReason_InterfaceModified;

//        proc_exec_reason.mpReasonObject = (void*)proc; // note: intf is available from proc
//        //
//        proc->ProcessExecute(proc_exec_reason);

//        intf->PropagateInterfaceSignals(all_ok);
//        //
//        // Step5: due to cascading of i/f redo Step4 if any ready i/f found, else Step6.
//        redo = TRUE; // in case values were propagated, OPT later
//        intf->NeedsCommit(TRUE);

//        intf->mInputsUpdated      = FALSE;
//        proc->mHasExecutedInitial = TRUE;

//        mDeltaCycle++;
//      }
//    }
//    mpProcessCurrent = 0;

//    if (mDeltaCycle >= cAsyncProcReadyLoopCountMax) assert(0);
//  }

//  // Step6: commit the modified async i/f's.
//  for (r=async_view->RecordSearchFirst(); r; r=async_view->RecordSearchNext())
//  {
//    proc = (Process*  )r->GetValueAsPointer(mpAttrProcess);
//    intf = (Interface*)r->GetValueAsPointer(mpAttrInterface);
//    assert(intf);

//    if (intf->NeedsCommit()) intf->Commit();
//  }

    // Step7: invoke all non-sync proc, flush Sync objs & repeat proc invocation as needed.
    while (1) {
      rr = mpCurrentSchedulerRecord = mpProcessesView->RecordCurrentGet();

      while (mpCurrentSchedulerRecord) {
        assert(mpProcessCurrent=proc=(Process*)(mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrProcess)));

        proc_type = proc->ProcessType();

        if (proc_type == eProcessType_Asynchronous) {
          assert(0); // re-enable logic below once 'r=async_view->Re,,,,' is uncommented above.
          proc = (Process*  )r->GetValueAsPointer(mpAttrProcess);
          intf = (Interface*)r->GetValueAsPointer(mpAttrInterface);
          assert(proc); assert(intf);

          Stack<ProcessExecuteReason>* rsn_stack;
          rsn_stack = (Stack<ProcessExecuteReason>*)(mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrExecReason));
          assert(rsn_stack);

          while (rsn_stack->Size()) {
            // Execute the process.
            proc->ProcessExecute(rsn_stack->Pop());
          }
          // Propagate process interface signals with async-rescheduling.
          intf->PropagateInterfaceSignals(all_ok, mSensitiveProcessList);
        }
        else if (proc_type == eProcessType_Timer
             ||  proc_type == eProcessType_Software)
        {
          mpCurrentSchedulerRecord->ValueCopyGet(mpAttrSimTime, iValSimTime);
          if ((unsigned)iValSimTime > mSimulationTime.TimeGet()) {
            mpProcessesView->RecordSearchPrev();
            break;
          }

          mCurrentRegion = (eSchedulerRegion)mpCurrentSchedulerRecord->GetValueAsUnsigned(mpAttrRegion);

          mScheduleEntriesProcessed++;

          Stack<ProcessExecuteReason>* rsn_stack;
          rsn_stack = (Stack<ProcessExecuteReason>*)(mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrExecReason));
          assert(rsn_stack);

          while (rsn_stack->Size()) {
            proc->ProcessExecute(rsn_stack->Pop());
          }

          // This is a good spot to reset: mUnblockedInCurrentDeltaCycle.
          proc->mUnblockedInCurrentDeltaCycle = FALSE;

          // No need to reschedule the timer/software process. If the process
          // wanted to be rescheduled then it would have been indicated to the
          // Scheduler when the process was executed. For example: the process
          // could have invoked mbox.ScheduleGet();
        }
        else {
          mpCurrentSchedulerRecord = mpProcessesView->RecordSearchPrev();
          break; // done with all non-sync processes (Software, Timer, Combinational)
        }

        mpCurrentSchedulerRecord = mpProcessesView->RecordSearchNext();
        p();
      }
      mpProcessCurrent = 0;

      // TBD: Add this:
      // delta_events += FlushAsyncProcesses();
      // unsigned FlushAsyncProcesses() {
      //   for (MacroListIterateAll(mSensitiveProcessList)) {
      //     Process* sens_proc = mSensitiveProcessList.Get();
      //     assert(sens_proc);
      //
      //     ScheduleSensitiveProcess(sens_proc);
      //   }
      // }

      // All simulation events in the current delta-cycle are flushed.
      delta_events  =  FlushTimers();
      delta_events +=  FlushEventsSet();
      delta_events +=  FlushSemaphoreGets();
      delta_events +=  FlushSemaphorePuts();
      delta_events +=  FlushMailboxPuts();

      mDeltaCycle++;

      // At the end of the delta cyle: mark processes that need to be blocked
      FlushProcessesToBlock();

      if (! delta_events) break;
      else {
        delta_events_total += delta_events;

        rr = mpCurrentSchedulerRecord = mpProcessesView->RecordSearchNext();
      }
    }

    // Step7a.  advance sim-time & clk edge to next scheduled sync process, end if none.
    // OR: should this be:
    // Step7a.  advance sim-time & clk edge to next scheduled      process, end if none.
    if (mUseInternalClock) {
      rr = mpCurrentSchedulerRecord = mpProcessesView->RecordSearchNext();

      // TBD: Ensure logic works even if the simulation has no Synchronous processes?
      if (mpCurrentSchedulerRecord) {
        mpCurrentSchedulerRecord->ValueCopyGet(mpAttrSimTime, iValSimTime);

        mSimulationTime       = (unsigned)iValSimTime;
        next_process_sim_time = mSimulationTime;

        // guarantee forward progress
        assert(mIsFirstSyncProcessExecuted==FALSE || mSimulationTime>*gSimTime);

        *gSimTime = mSimulationTime.TimeGet();
        mSimulationTimeStr = mSimulationTime.TimeGetStr();

        if (mSimulationTime >= mSimulationTimeMax) {
          mSchedulerState = eSchedulerState_SimTimeMaxReached;
          if (gDbg) printf("[%s] EndOfSim", mSimulationTime.TimeGetStr()), M_FL;

          break;
        }

        //if (gDbg) printf("[%u]\n", mSimulationTime);

        mIsFirstSyncProcessExecuted = TRUE;
        mScheduleEntriesProcessed++;
      }
      else {
        mSchedulerState = eSchedulerState_Finished;

        if (gDbg) printf("[%s] EndOfSim (no more scheduler entries)", mSimulationTime.TimeGetStr()), M_FL;

        break;
      }
    }
    else {
      // Step7b: wait for next clock indication from SystemVerilog simulation.
      // Once we can connect to a hardware simulator we should use this step.
      assert(0); // not supported for now
    }

    gVcd->DumpSimulationTime(mSimulationTime);

    intf_count = 0;

    // Step8: for all sync processes at the current sim. time and clock edge
    while (mpCurrentSchedulerRecord && (mSimulationTime==next_process_sim_time))
    {
      consider_count++;
      if (gDbg) { printf("Considering %d record: ", consider_count); mpCurrentSchedulerRecord->Print(); }
      // Step9: invoke process (use committed inputs from Step2 or StepB).
      proc      = (Process*)mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrProcess);
      proc_type = proc->ProcessType();
      clk       = 0;

      mpProcessCurrent = proc;

      if (proc_type != eProcessType_Synchronous
      &&  proc_type != eProcessType_Clock)
      {
        break;
      }

      if (proc->mProcessState != eProcessState_Running) {
        rr = mpCurrentSchedulerRecord = mpProcessesView->RecordSearchNext();

        goto SkipExecuteProcess;
      }

      clk = (Clock*)mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrClock);
      assert(clk);

      // CHECK: Is this needed?
      if (clk->ProcessStateGet() != eProcessState_Running
      ||  mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrSyncObj) != 0)
      {
        rr = mpCurrentSchedulerRecord = mpProcessesView->RecordSearchNext();

        goto SkipExecuteProcess;
      }

      intf = (Interface*)mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrInterface);
      assert(intf);

      // FIX: Reason not correct if process is a Clock.
      if (clk->ClockEdgeGet() == eSensitiveType_PositiveEdge)
        proc_exec_reason.mExecuteReason = eExecuteReason_ClockPosedge;
      else
        proc_exec_reason.mExecuteReason = eExecuteReason_ClockNegedge;
      //
      proc_exec_reason.mpReasonObject = (void*)clk;

      // Important: Here is where we actually execute a process.
      // printf("Executing: "); mpCurrentSchedulerRecord->Print();
      proc->ProcessExecute(proc_exec_reason);

      // StepA: Only commit for clock processes.
      if (proc->ProcessType() == eProcessType_Clock) {
        // Currently, clock and sync processes execute at the same time
        // separated only by region, commit now so that sync processes see
        // correct clock value.
        // Option to schedule clock's processes here if clock is not stopped.
        intf->PropagateInterfaceSignals(all_ok, mSensitiveProcessList);
      } else {
        // All other sync processes will have their interfaces committed together
        // at the end of this delta cycle (StepC).
        intf_to_commit[intf_count++] = intf;
      }

      // StepB: if not blocked, schedule process for next sync indication (clock edge).
      // Note: Another option is to keep a list of all sync processes in the
      //       corresponding clock and when the clock executes we schedule those
      //       processes. Maybe each clock can keep its processes in a view?
      if (mpCurrentSchedulerRecord->GetValueAsPointer(mpAttrSyncObj) == 0) {
        mScheduleTime = mSimulationTime;
        ScheduleProcessImpl(eScheduleRecord_Current, proc, mScheduleTime);

        // Update the current record after the relink operation.
        rr = mpCurrentSchedulerRecord = mpProcessesView->RecordCurrentGet();
      }
      else {
        rr = mpCurrentSchedulerRecord = mpProcessesView->RecordSearchNext();
      }

SkipExecuteProcess:
      if (mpCurrentSchedulerRecord) {
        mpCurrentSchedulerRecord->ValueCopyGet(mpAttrSimTime, iValSimTime);
        next_process_sim_time = (unsigned)iValSimTime;
      }
      mpProcessCurrent = 0;
    }
    // No more processes to run, proceed with commits, triggers etc.

    // StepC: commit all sync i/f's modified in Step8.
    for (int i=0; i<intf_count; i++) {
      intf_to_commit[i]->PropagateInterfaceSignals(all_ok, mSensitiveProcessList);
      intf_to_commit[i]->Commit();
    }

    // StepD: commit all tables in the database.
    gDB->DBTransactionsCommitAll();

    // StepE: invoke all triggers that were activated in StepC and StepD.
    for (MacroListIterateAll(mTriggerList)) {
      Trigger* trg = mTriggerList.Get();
      assert(trg);

      trg->TriggerActionExecute();
    }

    // Empty out the list without deleting the contained triggers.
    mTriggerList.DeleteLinkAll();
  }

  return TRUE;
}

void Scheduler::SimulationTimeMax(SimulatorTime& tm)
{
  mSimulationTimeMax = tm;
}

void Scheduler::SimulationTimeMax(unsigned tm)
{
  mSimulationTimeMax = tm;
}

void Scheduler::ProcessBlock(Process* process, void* sync_obj, eSyncArg sync_arg)
{
  process->mpBlockingAgentPending = sync_obj;
  process->mBlockAgentArgPending  = sync_arg;

  mProcessesToBlockList.Add(process);
}

bool Scheduler::ProcessUnblock(Process* process, void* sync_obj, eSyncArg sync_arg)
{
  Record* rec = process->mpRecord;
  assert(rec);

  if ((*rec)[mpAttrSyncObj] == sync_obj
  &&  (*rec)[mpAttrSyncArg] == sync_arg)
  {
    (*rec)[mpAttrSyncObj] = (void*)0;      // clear
    (*rec)[mpAttrSyncArg] = eSyncArg_None; //   "

    process->mUnblockedInCurrentDeltaCycle = TRUE;

    return TRUE;
  }

  return FALSE;
}

bool Scheduler::ProcessIsBlocked(Process* process)
{
  Record* rec = process->mpRecord;
  assert(rec);

  if ((*rec)[mpAttrSyncObj] != (void*)0 || process->mUnblockedInCurrentDeltaCycle)
    return TRUE;

  return FALSE;
}

// For all processes that were scheduled to be blocked, block them.
bool Scheduler::FlushProcessesToBlock()
{
  for (M_ListIterateAndDelete(mProcessesToBlockList, TRUE)) {
    Process* process = mProcessesToBlockList.Get();
    assert(process);
    assert(process->mpRecord);

    // Cannot have any existing pending agent, check.
    assert((*(process->mpRecord))[mpAttrSyncObj] == (void*)0);

    void*    block_agent     = process->mpBlockingAgentPending;
    unsigned block_agent_arg = process->mBlockAgentArgPending;

    Record& rec = *(process->mpRecord);

    rec[mpAttrSyncObj] = block_agent;
    rec[mpAttrSyncArg] = block_agent_arg;
  }

  return TRUE;
}

void Scheduler::MailboxAdd(MailboxBase* mbox_base)
{
  if (! mMboxList.Exists(mbox_base)) {
    mMboxList.Add(mbox_base);

    mMailboxEvaluate = TRUE;
  }
}

bool Scheduler::TriggerSchedule(Trigger* trg)
{
  return (mTriggerList.Add(trg));
}

void Scheduler::Print()
{
  printf("View records:"), M_FL;
  mpProcessesView->Print();
}

eSchedulerRegion Scheduler::RegionGet(Process* process, eSensitiveType clk_edge)
{
  eProcessType process_type = process->ProcessType();

  return RegionGet(process_type, clk_edge);
}

eSchedulerRegion Scheduler::RegionGet(eProcessType process_type, eSensitiveType clk_edge)
{
  eSchedulerRegion region;

  switch (process_type) {
    case eProcessType_Clock: region = eSchedulerRegion_1; break;
    case eProcessType_Synchronous:
      if (clk_edge == eSensitiveType_PositiveEdge
      ||  clk_edge == eSensitiveType_NegativeEdge
      ||  clk_edge == eSensitiveType_PositiveAndNegativeEdge)
      {
        region = eSchedulerRegion_2;
      }
      else if (clk_edge == eSensitiveType_PositiveLevel
           ||  clk_edge == eSensitiveType_NegativeLevel)
      {
        region = eSchedulerRegion_3;
      }
      else { assert(0); }
      break;
    case eProcessType_Timer:        region = eSchedulerRegion_4; break;
    case eProcessType_Software:     region = eSchedulerRegion_5; break;
    case eProcessType_Asynchronous: region = eSchedulerRegion_6; break;
    //
    default: assert(0);
  }

  return region;
}

void Scheduler::ScheduleProcessImpl(eScheduleRecord sched_type, Process* proc, SimulatorTime& sched_sim_time)
{
  Record*          rec;
  eSchedulerRegion region;
  eSensitiveType   clk_edge;
  unsigned         delta_cycle;

  // Existing records need to be searched.
  if (sched_type == eScheduleRecord_Existing) {
    impQueryProcFind = (*mpAttrProcess==(void*)proc); // *mpAttr_RecordPtr==(void*)rec
    rec=mpProcessesViewIterSearch2->RecordSearchFirst(impQueryProcFind);
    assert(rec == proc->mpRecord);
    sched_type = eScheduleRecord_FromIter;
  }

  rec = proc->mpRecord;

  if (proc->ProcessType() == eProcessType_Synchronous
  ||  proc->ProcessType() == eProcessType_Clock)
  {
    // get next clock time
    clk_edge        = (eSensitiveType)rec->GetValueAsUnsigned(mpAttrClockEdge);
    Clock* clk      = (Clock*)rec->GetValueAsPointer(mpAttrClock);
    sched_sim_time += clk->SimTimeIncrForNextClockEdge(clk_edge);
    assert(sched_sim_time > mSimulationTime);
    delta_cycle     = 0;
  }
  else {
    if (mSchedulerState != eSchedulerState_NotStarted)
      delta_cycle = mDeltaCycle + 1;
  }

  region = (eSchedulerRegion)proc->mpRecord->GetValueAsUnsigned(mpAttrRegion);

  ScheduleTheEntry(sched_type, sched_sim_time, delta_cycle, region, mpProcessesViewIterSearch2, proc->mpRecord);
  // TBD: Merge this function with ScheduleTheEntry()
}

void Scheduler::ScheduleTheEntry(eScheduleRecord sched_type, SimulatorTime& st_entry_sched_time, unsigned delta_cycle, eSchedulerRegion region, ViewIterator* iter_for_relink, Record* rec)
{
  unsigned  entry_sched_time = st_entry_sched_time.TimeGet();

  bool new_rec = (sched_type == eScheduleRecord_New);

  (*rec)[mpAttrSimTime] = st_entry_sched_time.TimeGet();

  // Insert the entry based on simulation time (lowest first, highest last)
  // and then the same way for region.

  // found=FALSE;
  // Note: We start scanning from the current record once sim is in progress.
  // for (i=0; rec.next() && !found; i++) {
  //   if      (t<rec.time)                  { found=TRUE; }
  //   else if (t==rec.time && r<rec.region) { found=TRUE; }
  // }
  // if      (i==0)  insert=none;   // when relinking the rec can already be in position
  // else if (found) insert=before;
  // else            insert=last;

  int     i;
  bool    found;
  //
  eListAddPosition pos_add;

  Record* rec_q  =0;
  Record* rec_dbg=0; // debug: entry is inserted just before this rec
  Record* rec_scd=0; // debug: record to be scheduled

  // If the Scheduler has started then we allow
  // adding records only after the current record.
  if (mSchedulerState == eSchedulerState_Started) {
    // Do not allow to  schedule something which is already in the past.
    assert(entry_sched_time >= mSimulationTime.TimeGet());

    mpProcessesViewIterSearch->PositionSetSameAsView();
    mpProcessesViewIterSearch->RecordSearchNext();
  }
  else {
    mpProcessesViewIterSearch->RecordSearchFirst();
  }

  found = FALSE;

  for (i=0; rec_q=mpProcessesViewIterSearch->RecordCurrentGet(); i++) {
    if      ((*rec_q)[mpAttrSimTime]    > entry_sched_time) found=TRUE;
    else if ((*rec_q)[mpAttrSimTime]    < entry_sched_time) ;
    else if ((*rec_q)[mpAttrDeltaCycle] > delta_cycle)      found=TRUE;
    else if ((*rec_q)[mpAttrDeltaCycle] < delta_cycle)      ;
    else if ((*rec_q)[mpAttrRegion]     > region)           found=TRUE;
    else                                                    ;

    if (found) break;
    if (! mpProcessesViewIterSearch->RecordSearchNext()) break;
  }

  if      (found && i==0 && !new_rec) pos_add = eListAddPosition_NoChange;
  else if (found)                     pos_add = eListAddPosition_BeforeCurrent;
  else                                pos_add = eListAddPosition_Last;

  if (found) rec_dbg = rec_q;

  if (sched_type == eScheduleRecord_New) {
    // Insert the new rec to the computed position.
    rec_scd = rec;
    mpProcessesView->RecordAdd(rec, pos_add, mpProcessesViewIterSearch);

  //fprintf(mSchedLogFp, "Adding: < %s>|<%d><name>\n", st_entry_sched_time.TimeGetStr(), region);
  }
  else if (pos_add == eListAddPosition_NoChange) {
    // Current record will be the scheduled record (scheduled time would have been updated).
    rec_scd = rec_q;
  }
  else if (sched_type == eScheduleRecord_Current) {
    assert(rec_q);
    rec_scd = rec_q;

    mpProcessesView->RecordMoveCurrToIter(mpProcessesViewIterSearch, pos_add,
     eListCurrentLinkAfterUnlink_Next);
  }
  else if (sched_type == eScheduleRecord_FromIter) {
    // Either move the current record Or the one indicated by 'iter_for_relink'.
    if (! iter_for_relink) {
      rec_scd = mpProcessesViewIterSearch->RecordCurrentGet();
      mpProcessesView->RecordMoveCurrToIter(mpProcessesViewIterSearch, pos_add,
       eListCurrentLinkAfterUnlink_Next);
    }
    else {
      rec_scd = iter_for_relink->RecordCurrentGet();

      if (found) {
        mpProcessesView->RecordMoveIterToIter(iter_for_relink, mpProcessesViewIterSearch,
         pos_add, eListCurrentLinkAfterUnlink_Next);
      }
      else {
        mpProcessesViewIterSearch->RecordSearchLast();
      //mpProcessesView->Print(mpProcessesViewIterSearch);

        mpProcessesView->RecordMoveIterToIter(iter_for_relink, mpProcessesViewIterSearch, eListAddPosition_AfterCurrent, eListCurrentLinkAfterUnlink_Default);
      }
    }
  }

  if (sDbg) {
    fprintf(mSchedLogFp, "Record  scheduling at: pos=%s",
     pos_add==0 ? "Add_First: " :
     pos_add==1 ? "Add_Before:" :
     pos_add==2 ? "Add_After: " :
     pos_add==3 ? "Add_Last:  " :
     pos_add==4 ? "NoChange:  " :
                  "Invalid:   "
    );
    rec_scd->Print(mSchedLogFp);

    if (rec_dbg) {
      fprintf(mSchedLogFp, "        relative to:                  ");
      rec_dbg->Print(mSchedLogFp);
    }
  }

  if (mSchedulerState == eSchedulerState_NotStarted) {
    // If the Scheduler has not started always make sure
    // the first record becomes the current record.
    mpProcessesView->RecordSearchFirst();
  }

  if (sLocalDbg) printf("Schedule entries at time %s are:\n", mSimulationTime.TimeGetStr());
  if (sLocalDbg) mpProcessesView->Print(-1);

  p();
}

// If a higher resolution timescale is specified adjust all the
// entries already in the Scheduler to reflect the new timescale.
// If a lower resolution timescale is specified we need only
// adjust that entry before scheduling it.
// Note: No more adjustments are possible after the Scheduler starts.
void Scheduler::AdjustTimeScale(eTimeScale timescale)
{
  assert(mSchedulerState == eSchedulerState_NotStarted);

  if (timescale > mSimulationTime.TimeScaleGet()) {
    // FIX: C does not have exponent operator.
    int multiplier = 1000^(timescale - mSimulationTime.TimeScaleGet());

    for (mpProcessesViewIterSearch->RecordSearchFirst();
          !mpProcessesViewIterSearch->RecordSearchIsDone();
           mpProcessesViewIterSearch->RecordSearchNext())
    {
      Record* rec = mpProcessesViewIterSearch->RecordCurrentGet();

      assert(rec);

      unsigned sim_time = rec->GetValueAsUnsigned(mpAttrSimTime);
      (*rec)[mpAttrSimTime] = sim_time * multiplier;
    }

    mSimulationTime.TimeScaleSet(timescale);
  }
}

unsigned Scheduler::FlushSemaphoreGets()
{
  unsigned found       = 0;
  bool     delete_link = FALSE;

  for (M_ListIterateAndDelete(mSemaphoreGetList, delete_link)) {
    Semaphore* sem = mSemaphoreGetList.Get();
    assert(sem);

    Process* process;

    while (process = sem->FlushGets()) {
      found++;

      Record* rec = process->mpRecord;

      // Add semaphore-reason to list-of-reasons.
      Stack<ProcessExecuteReason>* rsn_stack;
      rsn_stack = (Stack<ProcessExecuteReason>*)(rec->GetValueAsPointer(mpAttrExecReason));
      assert(rsn_stack);
      //
      impProcExecReason.mExecuteReason = eExecuteReason_SemaphoreGet;
      impProcExecReason.mpReasonObject = (void*)sem;
      rsn_stack->Push(impProcExecReason); // makes a copy
      fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
       process->Name(), Enum2Str(ExecuteReasonNV, impProcExecReason.mExecuteReason));
      // Schedule the event_proc
      mScheduleTime = mSimulationTime;
      ScheduleProcessImpl(eScheduleRecord_Existing, process, mScheduleTime);
    }

    if (sem->mSemInfoGetPending.Size()) delete_link = FALSE;
    else                                delete_link = TRUE;
  }

  return found;
}

unsigned Scheduler::FlushSemaphorePuts()
{
  unsigned found = 0;
  bool delete_link = FALSE;

  for (M_ListIterateAndDelete(mSemaphorePutList, delete_link)) {
    Semaphore* sem = mSemaphorePutList.Get();
    assert(sem);

    found++;

    Process* process;
    while (process = sem->FlushPuts()) {
      Record* rec = process->mpRecord;

      // Add semaphore-reason to list-of-reasons.
      Stack<ProcessExecuteReason>* rsn_stack;
      rsn_stack = (Stack<ProcessExecuteReason>*)(rec->GetValueAsPointer(mpAttrExecReason));
      assert(rsn_stack);
      //
      impProcExecReason.mExecuteReason = eExecuteReason_SemaphorePut;
      impProcExecReason.mpReasonObject = (void*)sem;
      rsn_stack->Push(impProcExecReason); // makes a copy
      fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
       process->Name(), Enum2Str(ExecuteReasonNV, impProcExecReason.mExecuteReason));

      // Schedule the Put process.
      mScheduleTime = mSimulationTime;
      ScheduleProcessImpl(eScheduleRecord_Existing, process, mSimulationTime);
    }

    if (sem->mSemInfoPutPending.Size()) delete_link = FALSE;
    else                                delete_link = TRUE;
  }

  return found;
}

unsigned Scheduler::FlushTimers()
{
  unsigned found = 0;

  // For all completed Timers unblock any of their processes.
  for (M_ListIterateAndDelete(mTimerList, TRUE)) {
    Timer* timer = mTimerList.Get();
    assert(timer);

    Stack<ProcessExecuteReason>* rsn_stack;
    Process* process;
    Record*  rec;

    while (timer->FlushTimer(&process)) {
      found++;

      rec = process->mpRecord;

      rsn_stack = (Stack<ProcessExecuteReason>*)(rec->GetValueAsPointer(mpAttrExecReason));
      assert(rsn_stack);

      impProcExecReason.mExecuteReason = eExecuteReason_TimerDone;
      impProcExecReason.mpReasonObject = (void*)timer;
      rsn_stack->Push(impProcExecReason); // makes a copy
      fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
       process->Name(), Enum2Str(ExecuteReasonNV, impProcExecReason.mExecuteReason));

      mScheduleTime = mSimulationTime;
      ScheduleProcessImpl(eScheduleRecord_Existing, process, mScheduleTime);
    }
  }

  return found;
}

unsigned Scheduler::FlushMailboxPuts()
{
  // If there has been no mailbox activity then we do not examine mailboxes.
  if (! mMailboxEvaluate) return FALSE;

  Process* proc_put;
  Process* proc_get;
  void*    elm;

  unsigned found = 0;

  for (M_ListIterateAll(mMboxList)) {
    MailboxBase* mbox = mMboxList.Get();
    assert(mbox);

    if (! mbox->FlushPutsAndGets(&proc_put, &proc_get, &elm)) continue;

    found++;

    Stack<ProcessExecuteReason>* rsn_stack_put = 0;
    Stack<ProcessExecuteReason>* rsn_stack_get = 0;
    //
    Record*  rec_put;
    Record*  rec_get;

    rec_put = (proc_put)->mpRecord;
    assert(rec_put);

    // Check if the Put needs to be Scheduled.
    if (proc_put) {
      // Add put-reason to list-of-reasons attribute
      rec_put = proc_put->mpRecord;

      rsn_stack_put = (Stack<ProcessExecuteReason>*)(rec_put->GetValueAsPointer(mpAttrExecReason));
      assert(rsn_stack_put);

      impProcExecReason.mExecuteReason = eExecuteReason_MailboxPut;
      impProcExecReason.mpReasonObject = (void*)mbox;

      rsn_stack_put->Push(impProcExecReason); // makes a copy
      fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
       proc_put->Name(), Enum2Str(ExecuteReasonNV, impProcExecReason.mExecuteReason));

      mScheduleTime = mSimulationTime;
      ScheduleProcessImpl(eScheduleRecord_Existing, proc_put, mScheduleTime);
    }

    assert(proc_get);

    // Schedule the Get().
    rec_get = proc_get->mpRecord;

    rsn_stack_get = (Stack<ProcessExecuteReason>*)(rec_get->GetValueAsPointer(mpAttrExecReason));
    assert(rsn_stack_get);

    impProcExecReason.mExecuteReason = eExecuteReason_MailboxGet;
    impProcExecReason.mpReasonObject = elm;

    rsn_stack_get->Push(impProcExecReason); // makes a copy
    fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
     proc_get->Name(), Enum2Str(ExecuteReasonNV, impProcExecReason.mExecuteReason));

    mScheduleTime = mSimulationTime;
    ScheduleProcessImpl(eScheduleRecord_Existing, proc_get, mScheduleTime);
  }

  mMailboxEvaluate = FALSE; // re-evaluate only for new Mailbox activity

  return found;
}

unsigned Scheduler::FlushEventsSet()
{
  unsigned found = 0;

  // For all 'set' events, check if any processes are waiting.
  while (mEventSetList.First()) {
    Event* event = mEventSetList.Get();
    assert(event);

    Process* process;
    while (process = event->Flush()) {
      found++;

      Record* rec = process->mpRecord;

      // Add event-reason to list-of-reasons attribute
      Stack<ProcessExecuteReason>* rsn_stack;
      rsn_stack = (Stack<ProcessExecuteReason>*)(rec->GetValueAsPointer(mpAttrExecReason));
      assert(rsn_stack);

      impProcExecReason.mExecuteReason = eExecuteReason_Event;
      impProcExecReason.mpReasonObject = (void*)event;
      rsn_stack->Push(impProcExecReason); // makes a copy

      fprintf(mSchedLogFp, "Process %s: Adding execute reason %s\n",
       process->Name(), Enum2Str(ExecuteReasonNV, impProcExecReason.mExecuteReason));

      mScheduleTime = mSimulationTime;
      ScheduleProcessImpl(eScheduleRecord_Existing, process, mScheduleTime);
    }

    mEventSetList.DeleteLinkThenIterationDone();
  }

  return found;
}

void Scheduler::SimulationTimeSet(SimulatorTime& tms)
{
  tms = mSimulationTime;
//mSimulationTime.TimeSet(tms);
}

// Quick print from debugger, doesnt like this function in the header file.
void Scheduler::p() { mpProcessesView->PrintHeader(); mpProcessesView->Print(); }

// End
