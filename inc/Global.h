// $RCSfile: Global.h,v $
// $Revision: 1.9 $

#ifndef GLOBAL
#define GLOBAL

#include <limits.h>

#define TRUE  1
#define FALSE 0

#define DONT_CARE 0

#ifndef ushort
#define ushort unsigned short
#endif

#define M_FL printf("\t@%s:%d\n", __FILE__, __LINE__)

#define M_ExternConstructDestructCountInt extern int gConstructCount[]; \
                                          extern int gDestructCount[];

#define M_UpdConstructCount(arg)         gConstructCount[arg] += 1;
#define M_UpdDestructCount(arg)          gDestructCount [arg] += 1;
#define M_UpdConstructCountN(arg, count) gConstructCount[arg] += count;
#define M_UpdDestructCountN(arg, count)  gDestructCount [arg] += count;

#define M_ConstructEnable 0

#define M_Constructed(arg)         if (M_ConstructEnable) \
                                     printf("%s constructed in %s %d\n", #arg, __FILE__, __LINE__)
#define M_ConstructedN(arg, count) if (M_ConstructEnable) \
                                     printf("%s %d constructed in %s %d\n", #arg, count, __FILE__, __LINE__)

#define MacroIsExactMultiple(A, B) !(A % B)
#define MacroAssert1(A)             { assert(A); }
#define MacroAssert2(A, B)          { assert(A); assert(B); }
#define MacroAssert3(A, B, C)       { MacroAssert2(A, B); assert(C); }
#define MacroAssert4(A, B, C, D)    { MacroAssert2(A, B) MacroAssert2(C,D) }
#define MacroAssert5(A, B, C, D, E) { MacroAssert2(A, B) MacroAssert3(C,D, E) }

enum eCompare { eCompareLess = -1, eCompareEqual, eCompareGreater, eCompareNotEqual };
enum          { NOT_OK,   OK   };
enum          { DIFFER,   SAME }; 
enum          { OFF,      ON   };
enum          { NOT_DONE, DONE };
enum          { eEnumInvalid=SHRT_MAX };

enum eSearchDirection { eForward, eBackward };

// Also update Enum.cc when adding enums.
enum eTypes   { eType_Aggregate,                    //  0
                eType_Attribute,                    //  1
                eType_Bit,                          //  2
                eType_Bool,                         //  3
                eType_Bucket,                       //  4
                eType_BucketList,                   //  5
                eType_Char,                         //  6
                eType_Clock,                        //  7
                eType_Condition,                    //  8
                eType_DBTransaction,                //  9
                eType_DBTransactionRec,             // 10
                eType_DBTransactionRecValue,        // 11
                eType_Database,                     // 12
                eType_DecodeEncode8b10b,            // 13
                eType_Event,                        // 14
                eType_EventFromTrigger,             // 15
                eType_Interface,                    // 16
                eType_Link,                         // 17
                eType_List,                         // 18
                eType_ListIterator,                 // 19
                eType_MailboxBase,                  // 20
                eType_Mailbox,                      // 21
                eType_Module,                       // 22
                eType_ModuleTop,                    // 23
                eType_Module_PhyUsbPcie,            // 24
                eType_Mutex,                        // 25
                eType_PRNG,                         // 26
                eType_Pointer,                      // 27
                eType_Process,                      // 28
                eType_ProcessAndEdgeInfo,           // 29
                eType_ProcessExecuteReason,         // 30
                eType_Process_PipeSerial,           // 31
                eType_Process_RW,                   // 32
                eType_Query,                        // 33
                eType_Record,                       // 34
                eType_Scheduler,                    // 35
                eType_Semaphore,                    // 36
                eType_SemaphoreInfo,                // 37
                eType_SensitiveInfo,                // 38
                eType_Short,                        // 39
                eType_Signal,                       // 40
                eType_Simulator,                    // 41
                eType_SimulatorTime,                // 42
                eType_Stack,                        // 43
                eType_String,                       // 44
                eType_Table,                        // 45
                eType_TableRecord,                  // 46
                eType_Timer,                        // 47
                eType_Trigger,                      // 48
                eType_Unsigned,                     // 49
                eType_Value,                        // 50
                eType_Vcd,                          // 51
                eType_View,                         // 52
                eType_ViewIterator,                 // 53
                eType_Max                           // 54
              };

#define MAX_DATA_BYTES           2048
#define MAX_FILE_LINE_LEN        256
#define MAX_FILENAME_LEN         512

#define MAX_LEN_FILELINE         256
#define MAX_LEN_FILENAME         512
#define MAX_FILENAME_LENGTH     1024

#define cUnsignedMax             UINT_MAX

#define CHECK_OBJS_DELETED \
bool check_objects_deleted(bool print, FILE* fp) \
{ \
  bool all_deleted_ok = TRUE; \
 \
  /* Memory allocation/deallocation check.*/ \
  /* Print out the cleanly deleted items. */ \
  for (int i=0, j=0; i<eType_Max; i++) { \
    if (gConstructCount[i] == gDestructCount[i] && gConstructCount[i] != 0) { \
      if (print) if (j==0) fprintf(fp, "Clean:\n"); \
      if (print) \
        fprintf(fp, "Eq:%2d  Type:%25s Constructed/Destructed=%d/%d\n", j++, Enum2Str(TypesNV, i), gConstructCount[i], gDestructCount[i]); \
    } \
  } \
 \
  /* Print out the not cleanly deleted items. */ \
  for (int i=0, j=0; i<eType_Max; i++) { \
    if (gConstructCount[i] != gDestructCount[i]) { \
      if (print) { \
        if (j == 0) fprintf(fp, "Not clean:\n"); \
        fprintf(fp, "Neq:%2d Type:%25s Constructed/Destructed=%d/%d\n", j++, Enum2Str(TypesNV, i), gConstructCount[i], gDestructCount[i]); \
      } \
      all_deleted_ok = FALSE; \
    } \
  } \
 \
  return all_deleted_ok; \
}

// Definitions to help convert Binary to Hex representation.
#define b_0 0x0
#define b_1 0x1

#define b_00 0x0
#define b_01 0x1
#define b_10 0x2
#define b_11 0x3

#define b_000 0x0
#define b_001 0x1
#define b_010 0x2
#define b_011 0x3
#define b_100 0x4
#define b_101 0x5
#define b_110 0x6
#define b_111 0x7

#define b_0000 0x0
#define b_0001 0x1
#define b_0010 0x2
#define b_0011 0x3
#define b_0100 0x4
#define b_0101 0x5
#define b_0110 0x6
#define b_0111 0x7
#define b_1000 0x8
#define b_1001 0x9
#define b_1010 0xa
#define b_1011 0xb
#define b_1100 0xc
#define b_1101 0xd
#define b_1110 0xe
#define b_1111 0xf

#endif

// End
