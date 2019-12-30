// $RCSfile: Vcd.h,v $
// $Revision: 1.2 $

#ifndef VcdClass
#define VcdClass

#include <stdio.h>

#include "String.h"
#include "Bit.h"
#include "Simulator.h"
#include "SimulatorTime.h"

class Interface;
class Module;
class Record;
class Signal;

enum eDumpState { eDumpNotStarted, eDumpStarted, eDumpPaused, eDumpResumed };

// Note:
// As a comparison, the following features are provided by verilog for dumping.
// - $dumpfile(filename);               // name of dumpfile
// - $dumpvars(level, list_of_modules); // no arg => all signals dumped
// - $dumpoff();                        // dump all signals as 'x' then stop
// - $dumpon();                         // dump all signals (checkpoint)
// - $dumplimit(int bytes);             // limit filesize at which dumping stops
// - $dumpflush();                      // flush the dump buffer

class Vcd
{
public:
  Vcd();
  ~Vcd();

  void DumpStart(Module* module, const char* dump_filename);
  void DumpPause();
  void DumpResume();
  bool DumpIsOn() { return mDumpState==eDumpStarted || mDumpState==eDumpResumed; }
  void DumpSignalDefinition(FILE* fp, Signal* signal);
  void DumpValueChange(char vcd_id[4], Bit* bit) const;
  void DumpSimulationTime(SimulatorTime& tm);

  eDumpState DumpStateGet() const { return mDumpState; }

  static const short cVcdSignalIdentifierStart = 33;  // Per verilog Spec.
  static const short cVcdSignalIdentifierEnd   = 126; //        "

private:
  String     mDumpFileName;
  eDumpState mDumpState;
  FILE*      mpFpDump;
  FILE*      mpFpNull;
  //
  char     mIdentifierGenerate0;
  char     mIdentifierGenerate1;
  char     mIdentifierGenerate2;
  char     mIdentifierCode[3];  // enough for (126-33)^3=830584 identifiers

  // Private member functions.
  void  VcdDefinitions(Module* module, bool is_first_pass);
  char* VcdSignalIdentifierNew();
};

#endif

// End
