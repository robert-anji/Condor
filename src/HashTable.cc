// $RCSfile: HashTable.cc,v $
// $Revision: 1.2 $

#include <assert.h>

static const unsigned sHashTableSizeInit = 16;

template<class T>
HashTable<T>::HashTable() :
  mSize                    (sHashTableSizeInit),
  mTotalEntries            (0),
  mCollisionSize           (sHashTableSizeInit>>3),
  mCollisionEntries        (0),
  mEntryCurrent            (0),
  mEntryCurrentIsCollision (FALSE),
  mEntriesRemoved          (FALSE),
  mIterationValid          (TRUE),
  mppHashTable             (0),
  mppHashTableCollisions   (0)
{
  mppHashTable = new T* [mSize];

  mppHashTableCollisions = new T* [mCollisionSize];
}

template<class T>
HashTable<T>::~HashTable()
{
  delete [] mppHashTable; mppHashTable = 0;

  delete [] mppHashTableCollisions; mppHashTableCollisions = 0;
}

template<class T>
void HashTable<T>::Add(T* t)
{
  mIterationValid = FALSE;

  // Compute hash, store in table[hash%size] if avail, else in collision
  unsigned index = Hash(t) % mSize;

  if (mppHashTable[index] == 0) {
    // No collision:
    mppHashTable[index] = t;

    mTotalEntries++;
  }
  else {
    // Collision:
    if (mCollisionEntries == mCollisionSize) {
      Resize();
    }

    mppHashTableCollisions[mCollisionEntries++] = t;

    mCollisionEntries++;
  }
}

template<class T>
bool HashTable<T>::Remove(T* t)
{
  mIterationValid = FALSE;

  // Compute hash, check table[hash%size]==v, else check in collision
  unsigned index = Hash(t) % mSize;

  if (mppHashTable[index] == t) {
    mppHashTable[index] = 0; // found, remove
  }
  else if (! mEntriesRemoved) {
    // If an entry was never removed then this hash never existed.
    return FALSE;
  }
  else {
    // Search in collision table:
    unsigned u;
    for (u=0; u<mCollisionEntries; u++) {
      if (mppHashTableCollisions[u] == t) {
        if (u > 0) mppHashTableCollisions[u] = mppHashTableCollisions[mCollisionEntries]; // found replace
        else       mppHashTableCollisions[u] = 0;

        break;
      }
    }

    if (u == mCollisionEntries) return FALSE;
  }

  mEntriesRemoved = TRUE;

  mTotalEntries--;
  return TRUE;
}

template<class T>
bool HashTable<T>::Exists(T* t)
{
  // Compute hash, check table[hash%size]==v, else check in collision
  unsigned index = Hash(t) % mSize;

  if (mppHashTable[index] == t) {
    return TRUE;
  }
  else if (! mEntriesRemoved) {
    // If an entry was never removed then this hash never existed.
    return FALSE;
  }
  else {
    // Search in collision table:
    for (unsigned u=0; u<mCollisionEntries; u++) {
      if (mppHashTableCollisions[u] == t) {
        return TRUE;
      }
    }
  }

  return FALSE;
}

template<class T>
void HashTable<T>::Clear()
{
  mIterationValid = FALSE;

  unsigned u;

  for (u=0; u<mSize; u++)
    mppHashTable[u] = 0;

  for (u=0; u<mCollisionSize; u++)
    mppHashTableCollisions[u] = 0;

  mTotalEntries = 0;
}

template<class T>
void HashTable<T>::Resize()
{
  unsigned additional_resize = FALSE;

  mIterationValid = FALSE;

  // Allocate temp tables of double size, recompute hash and re-store, copy over collision table entries.
  unsigned new_size = mSize*2;
  unsigned new_collision_size = new_size >> 3;

  T** new_hash_table           = new T* [new_size];
  T** new_hash_table_collision = new T* [new_collision_size];

  unsigned new_collision_count = 0;
  T* t;

  // Copy over previous hash table entries
  for (t=First(); t; t=Next()) {
    unsigned index = Hash(t) % new_size;

    if (new_hash_table[index] == 0) {
      new_hash_table[index] = t;
    }
    else {
      new_hash_table_collision[new_collision_count] = t;
      new_collision_count++;

      if (new_collision_count == new_collision_size) {
        // CONT: handle additional_resize
        additional_resize = TRUE;
      }
    }
  }

  // Copy over previous collision entries.
  for (unsigned u=0; u<mCollisionEntries; u++) {
    t = mppHashTableCollisions[u];

    unsigned index = Hash(t) % new_size;

    if (new_hash_table[index] == 0) {
      new_hash_table[index] = t;
    }
    else {
      new_hash_table_collision[new_collision_count] = t;
      new_collision_count++;

      if (new_collision_count == new_collision_size) {
        // CONT: handle additional_resize
        additional_resize = TRUE;
      }
    }
  }

  mppHashTableCollisions = new_hash_table_collision;

  delete [] mppHashTable;
  delete [] mppHashTableCollisions;

  mppHashTable           = new_hash_table;
  mppHashTableCollisions = new_hash_table_collision;

  mSize             = new_size;
  mCollisionSize    = new_collision_size;
  mCollisionEntries = new_collision_count;
  // mTotalEntries should stay the same
}

template<class T>
unsigned HashTable<T>::Hash(T* t)
{
  unsigned hash = 0;

  // SDBM Hash algorithm.
  hash = (t    )&255;//(hash << 6) + (hash << 16) - hash;
  hash = (t>> 8)&255 + (hash << 6) + (hash << 16) - hash;
  hash = (t>>16)&255 + (hash << 6) + (hash << 16) - hash;
  hash = (t>>24)&255 + (hash << 6) + (hash << 16) - hash;

  return hash;
}

template<class T>
T* HashTable<T>::First()
{
  mIterationValid = TRUE;

  mEntryCurrentIsCollision = FALSE;

  if (mTotalEntries) {
    for (unsigned u=0; u<mSize; u++) {
      if (mppHashTable[u]) {
        mEntryCurrent = u;

        return mppHashTable[u];
      }
    }

    for (unsigned u=0; u<mCollisionSize; u++) {
      if (mppHashTableCollisions[u]) {
        mEntryCurrent            = u;
        mEntryCurrentIsCollision = TRUE;

        return mppHashTableCollisions[u];
      }
    }
  }

  return 0;
}

template<class T>
T* HashTable<T>::Next()
{
  if (! mIterationValid) {
    assert(0);
  }

  if (! mEntryCurrentIsCollision) {
    for (unsigned u=mEntryCurrent+1; u<mSize; u++) {
      if (mppHashTable[u]) {
        mEntryCurrent = u;

        return mppHashTable[u];
      }
    }
  }
  else {
    if (mEntryCurrent+1 < mCollisionSize) {
      mEntryCurrent++;
      return mppHashTableCollisions[mEntryCurrent];
    }
  }

  return 0;
}

template<class T>
T* HashTable<T>::Prev()
{
  if (! mIterationValid) {
    assert(0);
  }

  if (mEntryCurrentIsCollision) {
    if (mEntryCurrent > 0) {
      mEntryCurrent--;
      return mppHashTableCollisions[mEntryCurrent];
    }
    else {
      mEntryCurrentIsCollision = FALSE;
      mEntryCurrent = mSize;
    }
  }

  if (mEntryCurrent != 0) {
    for (unsigned u=mEntryCurrent-1; u!=0; u--) {
      if (mppHashTable[u]) {
        mEntryCurrent = u;

        return mppHashTable[u];
      }
    }

    if (mppHashTable[0]) {
      mEntryCurrent = 0;

      return mppHashTable[0];
    }
  }

  return 0;
}

template<class T>
T* HashTable<T>::Current()
{
  if (! mIterationValid) {
    assert(0);
  }

  if (mEntryCurrentIsCollision) { return mppHashTableCollisions[mEntryCurrent]; }
  else                          { return mppHashTable[mEntryCurrent]; }
}

// End
