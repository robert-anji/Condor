// $RCSfile: DBDefines.h,v $
// $Revision: 1.4 $

#ifndef DBDefines_Header
#define DBDefines_Header

#include <limits.h>

#define MAX_ATTR_NAME_LENGTH    256
#define MAX_ATTR_TYPE_LENGTH    256
#define MAX_RECORD_LINE_LENGTH  1024
#define MAX_ATTR_VALUE_BYTES    1024
#define MAX_FILENAME_LENGTH     1024

enum eSortOrder {
  eSortOrder_Ascending,
  eSortOrder_Descending
};

enum eTableCommitMode {
  eTableCommitMode_Auto,
  eTableCommitMode_Manual
};

enum eTableOpenFlags {
  // Table open content:
  eTableOpenFlagBit_New             = 1<<0,
  eTableOpenFlagBit_FromFile        = 1<<1,
  eTableOpenFlagBit_Persistant      = 1<<2,
  // Table permissions:
  eTableOpenFlagBit_PermRead        = 1<<3,
  eTableOpenFlagBit_PermAppend      = 1<<4,
  eTableOpenFlagBit_PermModify      = 1<<5,
  eTableOpenFlagBit_PermDelete      = 1<<6,
  // Table commit mode:
  eTableOpenFlagBit_CommitManual    = 1<<7,
  eTableOpenFlagBit_UseTransactions = 1<<8,

  // Useful shortcuts:
  eTableOpenFlags_PermAll =
      eTableOpenFlagBit_New
    | eTableOpenFlagBit_PermRead
    | eTableOpenFlagBit_PermAppend
    | eTableOpenFlagBit_PermModify
    | eTableOpenFlagBit_PermDelete,
  //
  eTableOpenFlags_FromFile_PermAll =
      eTableOpenFlagBit_FromFile
    | eTableOpenFlagBit_PermRead
    | eTableOpenFlagBit_PermAppend
    | eTableOpenFlagBit_PermModify
    | eTableOpenFlagBit_PermDelete,
  //
  eTableOpenFlags_Default =
      eTableOpenFlags_PermAll
    | eTableOpenFlagBit_UseTransactions,
  //
  eTableOpenFlags_DontCare = 0
};

enum eRecordPermissionBits {
  // Owner permissions
  eRecPermBit_OwnerAdd      = 1<<0,
  eRecPermBit_OwnerDelete   = 1<<1,
  eRecPermBit_OwnerModify   = 1<<2,
  eRecPermBit_OwnerRead     = 1<<3,
  // Group permissions
  eRecPermBit_GroupAdd      = 1<<4,
  eRecPermBit_GroupDelete   = 1<<5,
  eRecPermBit_GroupModify   = 1<<6,
  eRecPermBit_GroupRead     = 1<<7,
  // Other permissions
  eRecPermBit_OtherAdd      = 1<<8,
  eRecPermBit_OtherDelete   = 1<<9,
  eRecPermBit_OtherModify   = 1<<10,
  eRecPermBit_OtherRead     = 1<<11,

  // Derived enums for frequently used permission combinations:
  eRecPerm_All =
      eRecPermBit_OwnerAdd    | eRecPermBit_GroupAdd    | eRecPermBit_OtherAdd
    | eRecPermBit_OwnerDelete | eRecPermBit_GroupDelete | eRecPermBit_OtherDelete
    | eRecPermBit_OwnerModify | eRecPermBit_GroupModify | eRecPermBit_OtherModify
    | eRecPermBit_OwnerRead   | eRecPermBit_GroupRead   | eRecPermBit_OtherRead,
  eRecPerm_OwnerGroupOrOtherAdd =
      eRecPermBit_OwnerAdd
    | eRecPermBit_GroupAdd
    | eRecPermBit_OtherAdd,
  eRecPerm_OwnerGroupOrOtherDelete =
      eRecPermBit_OwnerDelete
    | eRecPermBit_GroupDelete
    | eRecPermBit_OtherDelete,
  eRecPerm_OwnerGroupOrOtherModify =
      eRecPermBit_OwnerModify
    | eRecPermBit_GroupModify
    | eRecPermBit_OtherModify,
  eRecPerm_OwnerGroupOrOtherRead =
      eRecPermBit_OwnerRead
    | eRecPermBit_GroupRead
    | eRecPermBit_OtherRead,
  eRecPerm_OwnerAll =
      eRecPermBit_OwnerAdd
    | eRecPermBit_OwnerDelete
    | eRecPermBit_OwnerModify
    | eRecPermBit_OwnerRead,
  eRecPerm_GroupAll =
      eRecPermBit_GroupAdd
    | eRecPermBit_GroupDelete
    | eRecPermBit_GroupModify
    | eRecPermBit_GroupRead,
  eRecPerm_OtherAll =
      eRecPermBit_OtherAdd
    | eRecPermBit_OtherDelete
    | eRecPermBit_OtherModify
    | eRecPermBit_OtherRead,
  eRecPerm_None = 0
};

enum eValueType {
  eValueUnknown,
  //
  eValueBit,
  eValueChar,
  eValueCharPtr,
  eValueConstCharConstPtr,
  eValueVoidPtr,
  eValueString,
  eValueUnsigned,
  eValueInt,
  //
  eValueDontCare     = eValueUnknown,
  eValueTypeDontCare = eValueUnknown
};

enum eViewSelectOperation {
  eViewSelectUnique,
  eViewSelectDuplicate
};

enum eAggregateType {
  eAggr_ValueAverage,
  eAggr_ValueMinimum,
  eAggr_ValueMaximum,
  eAggr_ValueSum,
  eAggr_RecordsCount,
  eAggr_RecordsAdded,
  eAggr_RecordsDeleted,
  eAggr_None
};

enum eAggregateUpdateReasonType {
  eAggrUpdType_RecordAdd,
  eAggrUpdType_RecordDelete,
  eAggrUpdType_RecordModify,

  eAggrUpdType_RecordModifyPropagate
};

enum eTriggerType {
  eTrgUnconditional,
  eTrgRecAdd,
  eTrgRecDelete,
  eTrgRecModify,
  eTrgAggregate
//eTrgRecAttrModify
};

enum                  { eMaxRecords    = SHRT_MAX };
enum                  { eMaxAttributes = SHRT_MAX };

#endif

// End
