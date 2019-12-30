// $RCSfile: TestBit.cc,v $
// $Revision: 1.1 $

#include <assert.h>

#include "Bit.h"
#include "Global.h"
#include "Database.h"
#include "Enum.h"
#include "List.h"
#include "Simulator.h"
#include "ModuleTop.h"
#include "Vcd.h"

int            gConstructCount[eType_Max];
int            gDestructCount[eType_Max];
Bit           *gSavedBit;
Database*      gDB;
Simulator*     gSimulator;
SimulatorTime* gSimTime;
Vcd*           gVcd;

bool           gDbg = 1;

bool run_test();
bool check_objects_deleted(bool print, FILE* fp);

bool run_test()
{
  Bit b(10);

  b = 240;

  b[4][3] = b[8][7];

  Bit many_bits [10];

  Bit b1, b2, b3;
  Bit bb(8);

  b1 = 1<<30;
  b2 = 5;

  many_bits[0] = bb;
  many_bits[1] = 5;
  many_bits[1][22][3] = 1001;

  bb[3][2] = 2;
  bb.Print();

  bb << 2;
  bb.Print();

  return 1;
}

int main(int argc, char* argv[])
{
  bool test_result = run_test();

  // Memory check logic:
  bool print = TRUE;

  FILE* fp = NULL;
  if (print) fp = fopen("memory", "w");

  if (!check_objects_deleted(print, fp)) {
    printf("\nAll objects not deleted cleanly"), M_FL;
    if (!print) check_objects_deleted(print=TRUE, stdout);
  }
  else {
    printf("All objects deleted cleanly!!"), M_FL;
  }

  return test_result;
}

bool check_objects_deleted(bool print, FILE* fp)
{
  bool all_deleted_ok = TRUE;

  // Memory allocation/deallocation check.
  // Print out the cleanly deleted items.
  for (int i=0, j=0; i<eType_Max; i++) {
    if (gConstructCount[i] == gDestructCount[i] && gConstructCount[i] != 0) {
      if (print) if (j==0) fprintf(fp, "Clean:\n");
      if (print)
        fprintf(fp, "Eq:%2d  Type:%25s Constructed/Destructed=%d/%d\n", j++, Enum2Str(TypesNV, i), gConstructCount[i], gDestructCount[i]);
    }
  }

  // Print out the not cleanly deleted items.
  for (int i=0, j=0; i<eType_Max; i++) {
    if (gConstructCount[i] != gDestructCount[i]) {
      if (print) {
        if (j == 0) fprintf(fp, "Not clean:\n");
        fprintf(fp, "Neq:%2d Type:%25s Constructed/Destructed=%d/%d\n", j++, Enum2Str(TypesNV, i), gConstructCount[i], gDestructCount[i]);
      }
      all_deleted_ok = FALSE;
    }
  }

  return all_deleted_ok;
}

// End
