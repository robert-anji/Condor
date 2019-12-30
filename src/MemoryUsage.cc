// $RCSfile: MemoryUsage.cc,v $
// $Revision: 1.1 $

bool check_objects_deleted(bool print, FILE* fp);

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
        fprintf(fp, "Eq:%2d  Type:%25s Destructed/Constructed=%d/%d\n", 1+j++, Enum2Str(TypesNV, i), gDestructCount[i], gConstructCount[i]);
    }
  }

  /* Print out the not cleanly deleted items. */
  for (int i=0, j=0; i<eType_Max; i++) {
    if (gConstructCount[i] != gDestructCount[i]) {
      if (print) {
        if (j == 0) fprintf(fp, "Not clean:\n");
        fprintf(fp, "Neq:%2d Type:%25s Destructed/Constructed=%d/%d\n", 1+j++, Enum2Str(TypesNV, i), gDestructCount[i], gConstructCount[i]);
      }
      all_deleted_ok = FALSE;
    }
  }

  return all_deleted_ok;
}

// End
