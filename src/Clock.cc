// $RCSfile: Clock.cc,v $
// $Revision: 1.5 $

#include <assert.h>

#include "Clock.h"
//
#include "Global.h"
#include "Interface.h"
#include "InterfaceDesc_Clock.h"
#include "Process.h"
#include "Scheduler.h"
#include "SimulatorDefines.h"

extern Scheduler* gScheduler;
M_ExternConstructDestructCountInt;

// experiment
Clock& Clock::operator()(unsigned u) {
  mHalfPeriod = u;
  return *this;
}

Clock::Clock(const char* name, Module* parent_module, unsigned half_period, eTimeScale ts, eClockPhase phase, InterfaceDescription intf_clk[]) :
  Process                   (name, parent_module),
  mClockStartTime           (ts),
  mHalfPeriod               (half_period),
  mHalfPeriodScaled         (0), // scaled later
  mTimeScale                (ts),
  mClockPhaseStart          (phase),
  mClockPhase               (phase),
  mClockEdge                (eSensitiveType_Invalid),
  mTimeHasAdvanced          (FALSE),
  mClockScheduleSuspend     (FALSE),
  mBit                      (1)
{
  assert(mTimeScale != eTimeScale_Invalid);

  mBit = 0;

  mpInterface = new Interface(name, intf_clk);
  assert(mpInterface);

  Signal* signal = mpInterface->SignalGet(cClockName);
  assert(signal);

  mpInterface->SetOwningProcess(this);

  M_UpdConstructCount(eType_Clock);
}

Clock::~Clock()
{
  // mpInterface deleted in Process.

  M_UpdDestructCount(eType_Clock);
}

bool Clock::Start()
{
  mProcessState    = eProcessState_Running;

  mClockStartTime = gScheduler->SimulationTimeGet();
//gScheduler->SimulationTimeSet(mClockStartTime);

  assert(mClockStartTime.TimeScaleGet() >= mTimeScale);

  mHalfPeriodScaled = mHalfPeriod; // FIX: mHalfPeriod*(1000^(mClockStartTime.TimeScaleGet() - mTimeScale));

  unsigned tm_next;

  switch (mClockPhase) {
  case eClockPhase_JustBeforePositiveEdge: mClockEdge=eSensitiveType_PositiveEdge; mBit=0; tm_next=0;                 break;
  case eClockPhase_JustAfterPositiveEdge:  mClockEdge=eSensitiveType_NegativeEdge; mBit=1; tm_next=mHalfPeriodScaled; break;
  case eClockPhase_JustBeforeNegativeEdge: mClockEdge=eSensitiveType_NegativeEdge; mBit=1; tm_next=0;                 break;
  case eClockPhase_JustAfterNegativeEdge:  mClockEdge=eSensitiveType_PositiveEdge; mBit=0; tm_next=mHalfPeriodScaled; break;
  default:                                 assert(0);
  }

  mTimeHasAdvanced = FALSE;
}

extern SimulatorTime* gSimTime;

void Clock::ProcessExecute(ProcessExecuteReason& exec_reason)
{
  exec_reason.mExecutedOk = TRUE;

  if (mClockScheduleSuspend) {
    mProcessState = eProcessState_Suspended;
    // corner case: need to correctly set mTimeHasAdvanced on un-suspend.

    return;
  }

  mBit = !mBit;

  printf("ROB: [%u] Clk mBit=", gSimTime->TimeGet()); mBit.Print();

  Interface& clk_intf = *mpInterface;

//clk_intf[cClockName] = mBit;
  clk_intf[0] = mBit; // assumes only one signal for clocks

  if (mBit == 1) { mClockPhase = eClockPhase_JustAfterPositiveEdge; mClockEdge=eSensitiveType_PositiveEdge; }
  else           { mClockPhase = eClockPhase_JustAfterNegativeEdge; mClockEdge=eSensitiveType_NegativeEdge; }

  mTimeHasAdvanced = TRUE;
}

bool Clock::ProcessStateModify(eProcessState new_state)
{
  switch (new_state) {
  case eProcessState_Running:
    if (mProcessState == eProcessState_Suspended) {
      Start(); // restarts the clock and updates the process state
    }
    break;
  case eProcessState_Suspended:
    if (mProcessState == eProcessState_Running) {
      // Schedule the suspend for the next time the clock tries to execute
      // so that current sync processes still see the clock running.
      mClockScheduleSuspend = TRUE;
    }
    break;
  default:
    assert(0);
  }

  return TRUE;
}

eProcessType Clock::ProcessType() const
{
  return eProcessType_Clock;
}

unsigned Clock::SimTimeIncrForNextClockEdge(eSensitiveType next_clk_edge)
{
  unsigned sim_tm_incr = 0;

  if (! mTimeHasAdvanced) {
    // If the clock has just started, then we must consider its starting phase.
    switch (mClockPhaseStart) {
      case eClockPhase_JustBeforePositiveEdge:
        switch (next_clk_edge) {
          case eSensitiveType_PositiveEdge:           /*sim_tm_incr += 0*/          ; break;
          case eSensitiveType_PositiveLevel:          /*sim_tm_incr += 0*/          ; break;
          case eSensitiveType_PositiveAndNegativeEdge:/*sim_tm_incr += 0*/          ; break;
          case eSensitiveType_NegativeEdge:             sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_NegativeLevel:            sim_tm_incr += mHalfPeriod  ; break;
          default: assert(0);
        }
        break;
      case eClockPhase_JustAfterPositiveEdge:
        switch (next_clk_edge) {
          case eSensitiveType_PositiveEdge:             sim_tm_incr += mHalfPeriod*2; break;
          case eSensitiveType_PositiveLevel:          /*sim_tm_incr += 0*/          ; break;
          case eSensitiveType_PositiveAndNegativeEdge:  sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_NegativeEdge:             sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_NegativeLevel:            sim_tm_incr += mHalfPeriod  ; break;
          default: assert(0);
        }
        break;
      case eClockPhase_JustBeforeNegativeEdge:
        switch (next_clk_edge) {
          case eSensitiveType_PositiveEdge:             sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_PositiveLevel:            sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_PositiveAndNegativeEdge:/*sim_tm_incr += 0*/          ; break;
          case eSensitiveType_NegativeEdge:           /*sim_tm_incr += 0*/          ; break;
          case eSensitiveType_NegativeLevel:          /*sim_tm_incr += 0*/          ; break;
          default: assert(0);
        }
        break;
      case eClockPhase_JustAfterNegativeEdge:
        switch (next_clk_edge) {
          case eSensitiveType_PositiveEdge:             sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_PositiveLevel:            sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_PositiveAndNegativeEdge:  sim_tm_incr += mHalfPeriod  ; break;
          case eSensitiveType_NegativeEdge:             sim_tm_incr += mHalfPeriod*2; break;
          case eSensitiveType_NegativeLevel:          /*sim_tm_incr += 0*/          ; break;
          default: assert(0);
        }
        break;
      default: assert(0);
    }
  }
  else {
    eSensitiveType prev_clk_edge;

    if (mBit == 1 ) prev_clk_edge = eSensitiveType_PositiveEdge;
    else            prev_clk_edge = eSensitiveType_NegativeEdge;

    switch (next_clk_edge) {
      case eSensitiveType_PositiveEdge:
      case eSensitiveType_PositiveLevel:
        if (prev_clk_edge == eSensitiveType_PositiveEdge)
          sim_tm_incr = mHalfPeriodScaled<<1; // multiply by 2
        else
          sim_tm_incr = mHalfPeriodScaled;
        break;
      case eSensitiveType_NegativeEdge:
      case eSensitiveType_NegativeLevel:
        if (prev_clk_edge == eSensitiveType_PositiveEdge)
          sim_tm_incr = mHalfPeriodScaled;
        else
          sim_tm_incr = mHalfPeriodScaled<<1;
        break;
      case eSensitiveType_PositiveAndNegativeEdge:
        sim_tm_incr = mHalfPeriodScaled;
        break;
      default: assert(0);
    }
  }

  return sim_tm_incr;
}

// End
