#ifndef OBJECT_REUSE
#define OBJECT_REUSE

#include "Global.h"

template<class T>
class ObjectReuse
{
public:
  ObjectReuse();
 ~ObjectReuse();

  T*   New(bool clear=TRUE);
  void Free(T* t);

private:
  List<T> mObjFreeList;
  //
  unsigned mReuseCount; // number of objects reused
  unsigned mNewCount;   // number of objects allocated
  unsigned mAvailCount; // number of objects available for reuse
};

#include "ObjectReuse.cc"

#endif
