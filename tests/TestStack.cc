// $RCSfile: TestStack.cc,v $
// $Revision: 1.1 $

#include <stdio.h>

#include "Global.h"
//
int gConstructCount[eType_Max];
int gDestructCount[eType_Max];
//
bool check_objects_deleted(bool print, FILE* fp);
void show_memory_usage();

#include "Enum.h"
#include "ProcessExecuteReason.h"
#include "Stack.h"

void test_stack();

Stack<unsigned>              *stack_u;
Stack<ProcessExecuteReason>  *stack_p;
Stack<ProcessExecuteReason*> *stack_pp;

int main(int argc, char* argv[])
{
  for (unsigned u=0; u<eType_Max; u++) { gConstructCount[u]=gDestructCount[0]=0; }

  stack_u  = new Stack<unsigned>(10);
  stack_p  = new Stack<ProcessExecuteReason>(10);
  stack_pp = new Stack<ProcessExecuteReason*>(10);

  test_stack();

  delete stack_u;
  delete stack_p;
  delete stack_pp;

  show_memory_usage();

  return 1;
}

void test_stack()
{
  unsigned u, v;

  for (u=0; u<10; u++) stack_u->Push(u);

  for (v=0; v<10; v++) printf("Pop'd %u\n", stack_u->Pop());

  ProcessExecuteReason per[10];
  per[0].mExecutedOk = 1;

  for (u=0; u<10; u++)
    stack_p->Push(per[u]);

  for (u=0; u<10; u++) {
    ProcessExecuteReason* ptr;
    stack_pp->Push(ptr=&per[u]);
  }
}

void show_memory_usage()
{
  bool print = TRUE;

  FILE* fp = NULL;
  if (print) fp = fopen(".memory", "w");

  if (!check_objects_deleted(print, fp)) {
    printf("\nAll objects not deleted cleanly"), M_FL;
    if (!print) check_objects_deleted(print=TRUE, stdout);
  }
  else {
    printf("All objects deleted cleanly!!"), M_FL;
  }
}

bool check_objects_deleted(bool print, FILE* fp)
{
  bool all_deleted_ok = TRUE;

  /* Memory allocation/deallocation check.*/
  /* Print out the cleanly deleted items. */
  for (int i=0, j=0; i<eType_Max; i++) {
    if (gConstructCount[i] == gDestructCount[i] && gConstructCount[i] != 0) {
      if (print) if (j==0) fprintf(fp, "Clean:\n");
      if (print)
        fprintf(fp, "Eq:%2d  Type:%25s Constructed/Destructed=%d/%d\n", j++, Enum2Str(TypesNV, i), gConstructCount[i], gDestructCount[i]);
    }
  }

  /* Print out the not cleanly deleted items. */
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
