// $RCSfile: HashTable.h,v $
// $Revision: 1.2 $

#ifndef HashTableClass
#define HashTableClass

#include "Global.h"

class Value;

template<class T>
class HashTable
{
public:
  HashTable();
 ~HashTable();

  void Add    (T* t);
  bool Remove (T* t);
  bool Exists (T* t);
  void Clear  ();

  // Iterate.
  T*   First();
  T*   Next();
  T*   Prev();
  T*   Current();

  unsigned Count();

private:
  unsigned mSize;
  unsigned mTotalEntries;
  unsigned mCollisionSize;
  unsigned mCollisionEntries;
  unsigned mEntryCurrent;
  bool     mEntryCurrentIsCollision;
  bool     mEntriesRemoved;
  bool     mIterationValid;
  T**      mppHashTable;
  T**      mppHashTableCollisions;

  void Resize();
  unsigned Hash(T* t);
};

#include "HashTable.cc"

#endif

// End
