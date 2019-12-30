#include <assert.h>

#include "List.h"

template<class T>
ObjectReuse<T>::ObjectReuse() :
  mReuseCount  (0),
  mNewCount    (0),
  mAvailCount  (0)
{
}

template<class T>
ObjectReuse<T>::~ObjectReuse()
{
  for (M_ListIterateAll(mObjFreeList)) {
    T* t = mObjFreeList.Get();
    assert(t);

    delete t;
  }

  mReuseCount = 0;
  mNewCount   = 0;
  mAvailCount = 0;
}

template<class T>
T* ObjectReuse<T>::New(bool clear)
{
  T* t = (T*)0;

  if (mObjFreeList.First()) {
    t = mObjFreeList.Get();
    mObjFreeList.DeleteLinkThenNext();

    if (clear) t->Clear();

    mReuseCount++;
  }
  else {
    t = new T;

    mNewCount++;
  }

  return t;
}

template<class T>
void ObjectReuse<T>::Free(T* t)
{
  mObjFreeList.Add(t);

  mAvailCount++;
}

// End
