// $RCSfile:$
// $Revision:$

#ifndef Utilities
#define Utilities

// Connect two interfaces with different group names.
bool ConnectInterfaceGroup(Interface* intf1, const char* group_name1, Interface* intf2, const char* group_name2, eInterfaceConnectType type);

// Connect two interfaces having the same group name.
bool ConnectInterfaceGroup(Interface* intf1, Interface* intf2, const char* group_name, eInterfaceConnectType type);

#endif
