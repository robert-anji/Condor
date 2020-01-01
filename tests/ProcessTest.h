// $RCSfile: ProcessTest.h,v $
// $Revision: 1.1 $

#ifndef ProcessTestClass
#define ProcessTestClass

#include "Process.h"

class Clock;
class Interface;
class Module;

class ProcessTest : public Process
{
public:
  ProcessTest(const char* name, Interface* intf, Module* parent_module);
  ~ProcessTest();

  virtual void Execute(bool& all_ok);

private:
  short mProcessTestState;
  short mDelay;
};

#endif

// End
