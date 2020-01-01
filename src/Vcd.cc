// $RCSfile: Vcd.cc,v $
// $Revision: 1.3 $

#include <string.h>

#include "Vcd.h"
//
#include "Global.h"
#include "Interface.h"
#include "Module.h"
#include "Process.h"
#include "Record.h"

M_ExternConstructDestructCountInt;

Vcd::Vcd() :
  mDumpState           (eDumpNotStarted),
  mpFpDump             (0),
  mpFpNull             (0),
  mIdentifierGenerate0 (cVcdSignalIdentifierStart),
  mIdentifierGenerate1 (cVcdSignalIdentifierStart-1),
  mIdentifierGenerate2 (cVcdSignalIdentifierStart-1)
//mIdentifierCode      ({'\0', '\0', '\0', '\0'})
{
  M_UpdConstructCount(eType_Vcd);
}

Vcd::~Vcd()
{
  if (mpFpDump) { fclose(mpFpDump); mpFpDump = 0; }

  M_UpdDestructCount(eType_Vcd);
}

void Vcd::DumpStart(Module* module, const char* dump_filename)
{
  mDumpFileName = dump_filename;

  if (mDumpState == eDumpStarted
  ||  mDumpState == eDumpResumed)
  {
    return;
  }

  mDumpState = eDumpStarted;

  // Dump signals in vcd format, for example:
  //  $date Fri Dec 26 17:14:05 2008 $end
  //  $version Icarus Verilog        $end
  //  $timescale 1s                  $end
  //  $scope module tb_intc          $end
  //  $var reg 1 ! clk               $end
  //  $upscope                       $end
  //  $scope module tb_intc          $end
  //  $scope module int0             $end
  //  $var wire 1 ' irq              $end
  //  $var wire 32 ( irqs[31:0]      $end
  //  $upscope                       $end

  // Open file mDumpFileName.
  mpFpDump = fopen(mDumpFileName, "w");
  // Open a dummy file for the first pass.
  mpFpNull = fopen("/dev/null",   "w");

  bool is_first_pass;
  VcdDefinitions(module, is_first_pass=TRUE);
  VcdDefinitions(module, is_first_pass=FALSE);

  fclose(mpFpNull); mpFpNull = 0;

  // Subsequent signal value updates will occur
  // when the Scheduler commits the interfaces.
}

// Per the VCD format specification, we are allowed to use the same code for
// input signals as the output signal driving it. This works for Verdi but
// did not work on GtkWave.
// We need two passes, in the first pass all output signals are given a vcd
// code, inputs are skipped. In the second pass we assign the vcd code of
// inputs by re-using the vcd code of its connected output.
// However, we use a local variable to override this behavior.
void Vcd::VcdDefinitions(Module* module, bool is_first_pass)
{
  FILE* fp;
  bool  is_second_pass = !is_first_pass;
  bool  use_same_code  = FALSE; // set to true for Verdi and false for GtkWave,
                                // change to globally configurable option later
  bool  needs_pass2    = use_same_code;

  if (is_second_pass && !use_same_code) return; // second pass is only for 'same code'

  // Play a trick with fp, we dont want to print on pass1 if 2 passes are reqd.
  if      (is_first_pass && !needs_pass2) fp = mpFpDump;
  else if (is_first_pass &&  needs_pass2) fp = mpFpNull;
  else if (is_second_pass               ) fp = mpFpDump;

  if (module->ParentModule()==0) {
    // For topmost module (has no parent) print date, version and timescale.
    fprintf(fp, "$date\n    Mon Jan 01 00:00:01 2009\n$end\n");
    fprintf(fp, "$version\n    CondorSimulator 0.7\n$end\n");
    fprintf(fp, "$timescale\n    1ns\n$end\n");
  }

  // Create definitions for this module.
  fprintf(fp, "$scope module %s $end\n", module->Name());

  for (M_ListIterateAll(module->mProcessesList)) {
    // IMPR: Maybe let module return an interface list, makes Vcd independent of Process.
    Process* process = module->mProcessesList.Get();
    assert(process);

    Interface* intf = process->InterfaceGet();

    // Non sync/async processes have no interface, ignore them.
    if (intf == 0) continue;

    fprintf(fp, "$scope module %s $end\n", intf->Name());

    for (M_ListIterateAll(intf->mSignalList)) {
      Signal* signal = intf->mSignalList.Get();
      assert(signal);

      bool is_output;

      if (signal->IsDrivingSignal()) is_output = TRUE;
      else                           is_output = FALSE;

      if (is_first_pass && is_output || !use_same_code) {
        signal->VcdIdentifierSet(VcdSignalIdentifierNew());
      }

      if (is_second_pass && !is_output && use_same_code) {
        // Per the VCD format specification, we are allowed to use the
        // same code for input signals as the output signal driving it.
        Signal* s1 = signal->mConnectedSignalsList.Get();
        if (!s1) {
          printf("Error: signal %s is not driven", signal->Name()), M_FL;
          assert(0);
        }

        signal->VcdIdentifierSet(s1->VcdIdentifierGet());
      }

      if ((is_first_pass && !needs_pass2) || is_second_pass) DumpSignalDefinition(fp, signal);
   }

    // Second, create definitions for all sub-modules (call same function (recursion)).
    for (M_ListIterateAll(module->mSubModulesList)) {
      Module* sub_mod = module->mSubModulesList.Get();
      assert(sub_mod);

      VcdDefinitions(sub_mod, is_first_pass);
    }

    fprintf(fp, "$upscope $end\n");
  }

  if (module->ParentModule()==0) {
    // Print the end of definitions if this is the topmost module (no parent).
    fprintf(fp, "$upscope $end\n");
    fprintf(fp, "$enddefinitions $end\n");
    fprintf(fp, "#0\n");
    fprintf(fp, "$dumpvars\n");
  }
}

void Vcd::DumpPause()
{
  mDumpState = eDumpPaused;
}

void Vcd::DumpResume()
{
  mDumpState = eDumpResumed;
}

char* Vcd::VcdSignalIdentifierNew()
{
  // Vcd identifiers are one or more ascii characters with ascii codes
  // between 33 and 126 (inclusive), ie a range of 126-33+2 = 95 chars.

  // Generate a new unique identifier:
  // We allow a max of 3 ascii chars. We first generate all codes of 1 char,
  // then 2 chars and then 3 chars. Total combinations: 95 + 95*95 + 95*95*95.
  char vcd_code[4] = {'\0', '\0', '\0', '\0'};
  short i = mIdentifierGenerate0;
  short j = mIdentifierGenerate1;
  short k = mIdentifierGenerate2;

  short start = cVcdSignalIdentifierStart;
  short end   = cVcdSignalIdentifierEnd;

  if      (j==start-1) { vcd_code[0]=i++;                               }
  else if (k==start-1) { vcd_code[0]=i++; vcd_code[1]=j;                }
  else                 { vcd_code[0]=i++; vcd_code[1]=j; vcd_code[2]=k; }

  if (i==end+1) { i=start;   j++; }
  if (j==end+1) { j=start-1; k++; }
  if (k==end+1) { assert(0);      } // out of codes

  mIdentifierGenerate0 = i;
  mIdentifierGenerate1 = j;
  mIdentifierGenerate2 = k;
  strcpy(mIdentifierCode, vcd_code);

  return (char*)mIdentifierCode;
}

void Vcd::DumpSignalDefinition(FILE* fp, Signal* signal)
{
  short w = signal->Width();

  if (w == 1) fprintf(fp, "$var reg %d %s %s $end\n", w, signal->VcdIdentifierGet(), signal->Name());
  else        fprintf(fp, "$var reg %d %s %s [%d:0] $end\n", w, signal->VcdIdentifierGet(), signal->Name(), w-1);
}

void Vcd::DumpValueChange(char vcd_id[4], Bit* bit) const
{
  assert(bit);

  if (mpFpDump) {
    // Vcd format: first print the value then the identifier.
    bit->VcdPrint(mpFpDump);
    fprintf(mpFpDump, "%s\n", vcd_id);
  }
}

void Vcd::DumpSimulationTime(SimulatorTime& tm)
{
  if (mpFpDump) {
    if (tm != 0) fprintf(mpFpDump, "#%u\n", tm.TimeGetUnsigned());
    // else: #0 is already added as part of the header.
  }
}

// End
