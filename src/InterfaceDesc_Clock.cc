// $RCSfile: Interface_RW.cc,v $
// $Revision: 1.1 $

#include "Interface.h"

#include "InterfaceDesc_Clock.h"

const char* const cClockName  = "Clock";

InterfaceDescription InterfaceDesc_Clock[] = {
  eIntfClock,     cClockName, 1, " ", eSignalType_Output,  "CLK",
  eIntfClockEnd,  "Clock",    0, " ", eSignalType_Invalid, "",
};

// End
