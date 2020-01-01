// $RCSfile: SimulatorDefines.h,v $
// $Revision: 1.1 $

#ifndef SimulatorDefines_Header
#define SimulatorDefines_Header

enum eTimeScale {
  eTimeScale_s,
  eTimeScale_ms,
  eTimeScale_us,
  eTimeScale_ns,
  eTimeScale_ps,
  eTimeScale_fs,
  //
  eTimeScale_Default,
  //
  // Other spellings:
  eTimeScale_S  = eTimeScale_s,
  eTimeScale_MS = eTimeScale_ms,
  eTimeScale_US = eTimeScale_us,
  eTimeScale_NS = eTimeScale_ns,
  eTimeScale_FS = eTimeScale_fs,
  //
  eTimeScale_Invalid
};

enum eSensitiveType {
  eSensitiveType_None,
  eSensitiveType_PositiveEdge,
  eSensitiveType_PositiveLevel,
  eSensitiveType_NegativeEdge,
  eSensitiveType_NegativeLevel,
  eSensitiveType_PositiveAndNegativeEdge,
  eSensitiveType_ValueChange,
  eSensitiveType_Invalid
};

enum eClockScale {
  eClockScale_kHz,
  eClockScale_MHz,
  eClockScale_GHz,
  eClockScale_Invalid
};

enum eSyncArg {
  eSyncCallType_None,
  eSyncCallType_Put,
  eSyncCallType_Get,
  eSyncArg_None,
  eSyncArg_Get,
  eSyncArg_Put
};

enum eEdgePosition    { eEdgePosition_Before, eEdgePosition_After };

#endif

// End
