// $RCSfile: Enum.h,v $
// $Revision: 1.4 $

#ifndef ToolsHeader
#define ToolsHeader

typedef struct _nameval NameVal;
struct _nameval {
  short val;
  const char* name;
};

short       Str2Enum(  NameVal nv[], char* name);
bool        EnumExists(NameVal nv[], short _enum);
const char* Enum2Str(  NameVal nv[], short _enum);

extern NameVal OperatorNV[];
extern NameVal ValueTypeNV[];
extern NameVal Process_RW_StateNV[];
extern NameVal SignalEdgeNV[];
extern NameVal TypesNV[];

#endif

// End
