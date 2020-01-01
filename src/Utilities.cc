// $RCSfile:$
// $Revision:$

#include "Interface.h"
#include "Process.h"
#include "Signal.h"

bool ConnectInterfaceGroup(Interface* intf1, Interface* intf2, const char* group_name, eInterfaceConnectType type)
{
  return ConnectInterfaceGroup(intf1, group_name, intf2, group_name, type);
}

bool ConnectInterfaceGroup(Interface* intf1, const char* group_name1, Interface* intf2, const char* group_name2, eInterfaceConnectType type)
{
  MacroAssert4(intf1, group_name1, intf2, group_name2);

  bool ignore_group1 = FALSE;
  bool ignore_group2 = FALSE;

  int found_count = 0;

  if (!strlen(group_name1)) ignore_group1 = TRUE;
  if (!strlen(group_name2)) ignore_group2 = TRUE;

  bool error = FALSE;

  for (int i=0; !error; i++) {
    const char* s1 = intf1->mpInterfaceDescription[i].signal_name;
    const char* g1 = intf1->mpInterfaceDescription[i].signal_group;
    eSignalType t1 = intf1->mpInterfaceDescription[i].signal_type;
    bool   is_clk1 =    intf1->GetOwningProcess() // for clocks, intf & process are created together
                     && intf1->GetOwningProcess()->ProcessType()==eProcessType_Clock;

    if (!strlen(s1)) break; // no more names (signals) to connect

    // IMPR: g1 and g2 can describe multiple group names (separated by .).
    //       temp solution is to use strstr() below but that doesnt work if
    //       one group name is a sub-string of another group name.
    if (!ignore_group1 && !strstr(g1, group_name1)) continue;

    for (int j=0; !error; j++) {
      const char* s2 = intf2->mpInterfaceDescription[j].signal_name;
      const char* g2 = intf2->mpInterfaceDescription[j].signal_group;
      eSignalType t2 = intf2->mpInterfaceDescription[j].signal_type;
      bool   is_clk2 =    intf2->GetOwningProcess() // for clocks, intf & process are created together
                       && intf2->GetOwningProcess()->ProcessType()==eProcessType_Clock;

      if (!strlen(s2)) {
        printf("Error, signal %s not found in interface %s", s1, intf2->Name()), M_FL;
        assert(0); // no matching name (signal) found
        break;
      }

      if (!ignore_group2 && !strstr(g2, group_name2)) continue;

      // Signal names must match except for clock processes.
      // For clock processes the groups matching is sufficient.
      if (strcmp(s1, s2) && !is_clk1 && !is_clk2) continue;

      Signal* signal1 = intf1->SignalGet(s1);
      Signal* signal2 = intf2->SignalGet(s2);
      MacroAssert2(signal1, signal2);

      // When we create an interface with signal directions reversed the clock
      // input also gets reversed and becomes an output, we fix this here
      // (behind the scenes) because we can now assume the signal is meant to
      // be a clock input.
      if (is_clk1) signal2->MarkAsInputIfReversed();
      if (is_clk2) signal1->MarkAsInputIfReversed();

      // Either intf1 or intf2 can be used to connect the signals.
      intf1->Connect(signal1, signal2, error);

      if (error) {
        printf("Error: Not able to connect %s with %s", s1, s2), M_FL;
        assert(0);
      }
      else {
      //if (is_clk1) intf2->SetAsSynchronous(); // TBD
      //if (is_clk2) intf1->SetAsSynchronous(); // TBD

        found_count++;
        break;
      }
    }
  }
  
  return (error==FALSE && found_count>0);
}

// End
