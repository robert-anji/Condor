// $RCSfile: Stack.h,v $
// $Revision: 1.3 $

#ifndef StackClass
#define StackClass

#include <assert.h>

#include "Global.h"

M_ExternConstructDestructCountInt;

template <class T>
class Stack
{
public:
  Stack(unsigned max_entries=64);
 ~Stack();

  bool Push(T& elem);
  T&   Pop ();
  bool Find(T& elem);

  unsigned Size() const { return mEntries; }

private:
  T*       mpEntries;
  unsigned mEntriesMax;
  unsigned mEntries;
  unsigned mHeadPtr;
  unsigned mTailPtr;
};

template <class T>
Stack<T>::Stack(unsigned max_entries) :
  mEntriesMax  (max_entries),
  mEntries     (0),
  mpEntries    (0),
  mHeadPtr     (0),
  mTailPtr     (0)
{
  if (mEntriesMax) {
    mpEntries = new T [mEntriesMax];
    assert(mpEntries);
  }

  M_UpdConstructCount(eType_Stack);
}

template <class T>
Stack<T>::~Stack()
{
  delete [] mpEntries; mpEntries = 0;

  M_UpdDestructCount(eType_Stack);
}

// Note:
// When T is a pointer then we need to invoke Push() with an intermediate
// reference variable as shown below:
// > stack_pp = new Stack<ProcessExecuteReason*>(10);
// > ProcessExecuteReason  per[10];
// > ProcessExecuteReason* ptr;
// > stack_pp->Push(ptr=&per[u]);
// Directly using &per[u] will not compile:
// > stack_pp->Push(&per[u]); // does not compile
//
template <class T>
bool Stack<T>::Push(T& elem)
{
  // IMPR: allow stack to grow dynamically (resize).
  assert(mEntries < mEntriesMax);

  mpEntries[mHeadPtr++] = elem;

  if (mHeadPtr == mEntriesMax) mHeadPtr = 0;

  mEntries++;

  return TRUE;
}

template <class T>
T& Stack<T>::Pop()
{
  assert(mEntries);

  unsigned tail = mTailPtr;

  mEntries--;
  mTailPtr++;

  if (mTailPtr == mEntriesMax)
    mTailPtr = 0;

  return mpEntries[tail];
}

template <class T>
bool Stack<T>::Find(T& elem)
{
  unsigned u, t;

  for (u=0, t=mTailPtr; u<mEntries; u++) {
    if (mpEntries[t] == elem) return TRUE;

    if (t++ == mEntriesMax) t = 0;
  }

  return FALSE;
}

#endif

// End
