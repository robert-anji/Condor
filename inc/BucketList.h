// $RCSfile: BucketList.h,v $
// $Revision: 1.4 $

#ifndef BucketListClass
#define BucketListClass

#include <assert.h>
#include <stdio.h>

// Template for a bucket list.
// Classes that want to use this list need to define a copy constructor.

#include "Global.h"
#include "Database.h"

M_ExternConstructDestructCountInt;

enum ePosition {ePositionFirst, ePositionNext, ePositionCurrent,
                ePositionLast,  ePositionDone                  };

#define MacroBucketList_BucketIterateAll(bkt_list) \
  bkt_list.BktSetPosition(ePositionFirst); !bkt_list.BktDone(); bkt_list.BktSetPosition(ePositionNext)

#define MacroBucketList_ListIterateAll(bkt_list) \
  bkt_list.SetLinkPosition(ePositionFirst); !bkt_list.LinkDone(); bkt_list.SetLinkPosition(ePositionNext)

template<class T>
class BktLink
{
public:
  BktLink()           : mpLinkNext(0)    {}
  BktLink(BktLink* link) : mpLinkNext(link) {}

  BktLink* mpLinkNext;
  T*    mpElem;
};

template<class T>
class Bucket
{
public:
  enum eBkt_CopyType { eBkt_CopyShallow, eBkt_CopyDeep };

  Bucket();
  ~Bucket();

  // Rename to CopyLinksOnly(), CopyLinksAndElems.
  void CopyShallow  (Bucket<T>& bkt) { return Copy(bkt, eBkt_CopyShallow); }
  void CopyDeep     (Bucket<T>& bkt) { return Copy(bkt, eBkt_CopyDeep);    }

  void Add     (T *elem, eAddDirection dir=eEast);
  void Clear();
  void Detach  ();
  T*   Get     (ePosition  pos);

  Bucket*      mpBktNext;
  BktLink<T>*  mpLinkFirst;
  BktLink<T>*  mpLinkLast;
  BktLink<T>*  mpLinkCurrent;
  int          mLinkLength;
private:
  void Copy(Bucket<T>& bkt, eBkt_CopyType type);
};

template<class T>
Bucket<T>::Bucket() :
  mpBktNext     (0),
  mpLinkFirst   (0),
  mpLinkLast    (0),
  mLinkLength   (0),
  mpLinkCurrent (0)
{
  M_UpdConstructCount(eType_Bucket);
}

template<class T>
Bucket<T>::~Bucket()
{
  BktLink<T>* next;

  // Only the links are deleted, elements must be deleted by the Bucket client.
  while (mpLinkFirst) {
    next = mpLinkFirst->mpLinkNext;

    delete mpLinkFirst; mpLinkFirst = 0;

    mpLinkFirst = next;
  }

  M_UpdDestructCount(eType_Bucket);
}

template<class T>
void Bucket<T>::Add(T *elem, eAddDirection dir)
{
  BktLink<T> *link = new BktLink<T>;

  link->mpElem     = elem;
  link->mpLinkNext = 0;
  
  if (dir == eEast) {
    if (mpLinkFirst) {
      mpLinkLast->mpLinkNext = link;
      mpLinkLast             = link;
    }
    else {
      mpLinkCurrent = mpLinkFirst = mpLinkLast = link;
    }
  }
  else {
    if (!mpLinkFirst) mpLinkCurrent = link;

    link->mpLinkNext = mpLinkFirst;
    mpLinkFirst      = link;

    if (!mpLinkLast) mpLinkLast = link;
  }
}

template<class T>
T* Bucket<T>::Get(ePosition pos)
{
  switch (pos) {
    case ePositionFirst:
      mpLinkCurrent = mpLinkFirst;

      if (mpLinkFirst) return mpLinkFirst->mpElem;
      else             return 0;

    case ePositionCurrent:
      return mpLinkCurrent->mpElem;

    case ePositionLast:
      mpLinkCurrent = mpLinkLast;

      if (mpLinkLast) return mpLinkLast->mpElem;
      else            return 0;

    case ePositionNext:
      if (mpLinkCurrent) {
        mpLinkCurrent = mpLinkCurrent->mpLinkNext;

        if (mpLinkCurrent) return mpLinkCurrent->mpElem;
      }
      return 0;

    case ePositionDone:
      return 0;

    default: fprintf(stdout, "Bucket Error: Cannot get for pos %d\n", pos);
  }
}

template<class T>
void Bucket<T>::Copy(Bucket<T>& bkt, eBkt_CopyType copy_type)
{
  Clear();

  T* elem = bkt.Get(ePositionFirst);

  while (elem) {
    if (copy_type == eBkt_CopyShallow) {
      Add(elem);
    }
    else if (copy_type == eBkt_CopyDeep) {
      T* elem_copy = new T(*elem);

      if (elem_copy) Add(new T(*elem));
      else           assert(0);
    }
    else assert(0);

    elem = bkt.Get(ePositionNext);
  }
}

// TBD: Rename to DeleteLinksAndElems().
template<class T>
void Bucket<T>::Clear()
{
  BktLink<T>* next;
  BktLink<T>* link = mpLinkFirst;

  while (link) {
    delete link->mpElem;
    next = link->mpLinkNext;
    delete link;
    link = next;
  }

  mpLinkFirst = mpLinkLast = 0;
  mLinkLength = 0;
  next   = 0;
}

template<class T>
void Bucket<T>::Detach()
{
  // This task is used to detach the links from the bucket, effectively
  // the memory management of the links and elements is now handled by
  // the caller of this task.
  mpBktNext      = 0;
  mpLinkFirst    = 0;
  mpLinkLast     = 0;
  mpLinkCurrent  = 0;
  mLinkLength    = 0;
}

template<class T>
class BucketList
{
public:
  enum eBktList_CopyType { eBktList_CopyShallow, eBktList_CopyDeep };

  BucketList();
  ~BucketList();

  // Add elem's or buckets to the bucket list.
  bool       Add(T* elm, eAddDirection dir);
  void       BktAdd(Bucket<T> *bkt, ePosition pos=ePositionNext);

  bool       BktSetPosition(ePosition pos);
  ePosition  BktGetPosition() const;
  bool       BktDone()        const;
  void       BktAppend(BucketList<T> &bl);

  // Bucket get
  Bucket<T>* BktGet(ePosition pos);

  // BktLink iterator
  bool      NextLink();
  //
  bool      SetLinkPosition(ePosition);
  ePosition GetLinkPosition();
  bool      LinkDone();

  // Assigment operator, used to make a copy.
  void CopyShallow(BucketList<T>& bl) { return Copy(bl, eBktList_CopyShallow); }
  void CopyDeep   (BucketList<T>& bl) { return Copy(bl, eBktList_CopyDeep); }

  // Attach one bucket list to another.
  // This will destroy the bucket list being attached.
  void AttachDestroy(BucketList<T>& bl, eAddDirection dir);
 
  // Other
  void Clean();
  void Print(FILE* fp);
  void p();
  T*   Get();

  short TotalBuckets() { return mBucketTotal;   }
  short TotalElems()   { return mElemTotal;     }
  short Size()         { return TotalBuckets(); }
  static const bool dbg=TRUE;
private:
  bool     AddLink(Bucket<T>* bkt, T* elem, eAddDirection dir);
  void     Copy(BucketList<T>& bl, eBktList_CopyType copy_type);

  Bucket<T>* mpBktFirst;
  Bucket<T>* mpBktLast;
  Bucket<T>* mpBktCurrent;
  BktLink<T>*   mpLinkCurrent;
  //
  short      mElemTotal;
  short      mBucketTotal;
  short      mBktDone;
  short      mLinkDone;

#ifdef Caching
  BktLink<T> *cachedLink;
  short cachedPos;
#endif
};

template<class T>
BucketList<T>::BucketList() :
#ifdef Caching
  cachedPos(0),
  cachedLink(0),
#endif
  mElemTotal(0),
  mpBktFirst(0),
  mpBktLast(0),
  mpLinkCurrent(0),
  mpBktCurrent(0),
  mBucketTotal(0),
  mBktDone(TRUE),
  mLinkDone(TRUE)
{
  M_UpdConstructCount(eType_BucketList);
}

template<class T>
BucketList<T>::~BucketList()
{
  Clean();

  M_UpdDestructCount(eType_BucketList);
}

template<class T>
void BucketList<T>::Clean()
{
  // Delete all links and buckets (but not the elements).

  Bucket<T>* next_bkt;

  while (mpBktFirst)
  {
    next_bkt = mpBktFirst->mpBktNext;

    delete mpBktFirst;

    mpBktFirst = next_bkt;
  }

  mElemTotal     = 0;
  mpBktFirst     = 0;
  mpBktLast      = 0;
  mpLinkCurrent  = 0;
  mpBktCurrent   = 0;
  mBucketTotal   = 0;
  mBktDone       = TRUE;
  mLinkDone      = TRUE;
}

template<class T>
void BucketList<T>::Print(FILE* fp)
{
  short count = 0;
  Bucket<T> *bkt;

  bkt = mpBktFirst;

  while (bkt)
  {
    fprintf(fp, "Bucket: %d\n", ++count);

    BktLink<T> *chain;

    chain = bkt->mpLinkFirst;

    while (chain)
    {
      chain->Elem->Print(fp);
      chain = chain->mpLinkNext;
    }

    bkt = bkt->mpBktNext;
  }

  fprintf(fp, "End BucketList Print\n");
}

template<class T>
void BucketList<T>::p()
{
  short count = 0;
  Bucket<T> *bkt;

  bkt = mpBktFirst;

  while (bkt)
  {
    fprintf(stdout, "Bucket %d: ", ++count);

    BktLink<T> *chain;

    chain = bkt->mpLinkFirst;

    while (chain)
    {
      chain->mpElem->Name(stdout);
      chain = chain->mpLinkNext;

      fprintf(stdout, " ");
    }
    fprintf(stdout, "\n");

    bkt = bkt->mpBktNext;
  }
}

template<class T>
void BucketList<T>::Copy(BucketList<T>& bl, eBktList_CopyType copy_type)
{
  Clean();

  if (bl.BktSetPosition(ePositionFirst))
  {
    while (!bl.BktDone()) {
      if (bl.SetLinkPosition(ePositionFirst))
      {
        T* elem = bl.Get();

        if (copy_type == eBktList_CopyShallow) {
          Add(elem, eSouth);
        }
        else if (copy_type == eBktList_CopyDeep) {
          T* elem_copy = new T(*elem);

          if (elem_copy) Add(elem_copy, eSouth);
          else           assert(0);
        }
        else assert(0);

        bl.SetLinkPosition(ePositionNext);

        while (!bl.LinkDone())
        {
          Add(bl.Get(), eEast);
          bl.SetLinkPosition(ePositionNext);
        }
      }

      bl.BktSetPosition(ePositionNext);
    }
  }
}

template<class T>
bool BucketList<T>::Add(T* elem, eAddDirection dir=eSouth)
{
  switch (dir)
  {
  case eNorth:
    if (mBucketTotal) {
      mpBktCurrent            = mpBktFirst = new Bucket<T>;
      mpBktCurrent->mpBktNext = mpBktFirst;
    }
    else {
      mpBktCurrent            = mpBktFirst = mpBktLast = new Bucket<T>;
      mpBktCurrent->mpBktNext = 0;
    }

    mBucketTotal++;
    mBktDone   = FALSE;
    mLinkDone  = FALSE;

    return AddLink(mpBktFirst, elem, eEast);
  case eSouth:
    if (mBucketTotal) {
      mpBktCurrent            = mpBktLast = mpBktLast->mpBktNext = new Bucket<T>;
      mpBktCurrent->mpBktNext = 0;
    }
    else {
      mpBktCurrent = mpBktLast = mpBktFirst = new Bucket<T>;
      mpBktCurrent->mpBktNext = 0;
    }

    mBucketTotal++;
    mBktDone   = FALSE;
    mLinkDone  = FALSE;

    return AddLink(mpBktLast, elem, eEast);
  case eEast:
    if (mpBktCurrent) {
      mBktDone   = FALSE;
      mLinkDone  = FALSE;

      return AddLink(mpBktCurrent, elem, eEast);
    }
    else
      return Add(elem, eSouth);

  case eWest:
    if (mpBktCurrent) {
      mBktDone   = FALSE;
      mLinkDone  = FALSE;

      return AddLink(mpBktCurrent, elem, eWest);
    }
    else
      return Add(elem, eSouth);
  default:
    return FALSE;
  }

  return FALSE;
}

template<class T>
bool BucketList<T>::AddLink(Bucket<T>* bkt, T* elem, eAddDirection dir)
{
  BktLink<T>* link;
  switch (dir)
  {
  case eWest:
    bkt->mpLinkFirst = new BktLink<T>(bkt->mpLinkFirst);
    link             = bkt->mpLinkFirst;
    break;
  case eEast:
    if (bkt->mpLinkLast)
      bkt->mpLinkLast = bkt->mpLinkLast->mpLinkNext = new BktLink<T>(0);
    else
      bkt->mpLinkLast = new BktLink<T>(0);

    link = bkt->mpLinkLast;
    break;
  default:
    return FALSE;
  }

  link->mpElem = elem;
  bkt->mLinkLength++;
  mElemTotal++;

  if (bkt->mLinkLength == 1)
    bkt->mpLinkFirst = bkt->mpLinkLast = link;

  return TRUE;
}

template<class T>
void BucketList<T>::BktAdd(Bucket<T> *bkt, ePosition pos)
{
  if (!bkt) return; // cannot add a null bucket

  switch (pos) {
    case ePositionNext:
      if (mpBktCurrent) {
        bkt->mpBktNext          = mpBktCurrent->mpBktNext;
        mpBktCurrent->mpBktNext = bkt;
        // Ensure subsequent adds will keep the 'add' order
        mpBktCurrent            = bkt;
      }
      else {
        fprintf(stdout, "BucketList Error: Cannot add bkt for next pos\n");
      }
      break;
    case ePositionFirst:
      if (mpBktFirst) {
        bkt->mpBktNext  = mpBktFirst;
        mpBktFirst      = bkt;
      }
      else {
        mpBktFirst = mpBktLast = bkt;
      }
    case ePositionLast:
      if (mpBktLast) {
        mpBktLast->mpBktNext = bkt;
        mpBktLast            = bkt;
        bkt->mpBktNext       = 0;
      }
      else {
        mpBktFirst = mpBktLast = bkt;
      }
    default:
      fprintf(stdout, "BucketList Error: Cannot add bkt for pos %d\n", pos);
  }
}

template<class T>
void BucketList<T>::AttachDestroy(BucketList<T>& bl, eAddDirection dir)
{
  // This unlinks all elements in bucket list bl and attaches
  // them to this bucket list (south or east), bl is consequently left empty.
  // Thus it is a destructive call for bl.
  // Used to avoid copying all bl elements into this bucket list.
  // This function must be used with care as it sets bl to the clear state.

  // Only pointers are manipulated as we are avoiding copying elements.
  if (bl.mpBktLast != 0)
  {
    if (dir == eSouth)
    {
      if (mpBktLast) {
        // both bkts nonempty
        mpBktLast->mpBktNext = bl.mpBktFirst;
        mpBktLast            = bl.mpBktLast;
      }
      else {
        // this bkt list is empty
        mpBktLast       = bl.mpBktLast;
      }

      mBktDone      = FALSE;
      mpBktCurrent  = bl.mpBktFirst;
      mBucketTotal += bl.mBucketTotal;
    }
    else if (dir == eEast)
    {
      BktSetPosition(ePositionFirst);
      bl.BktSetPosition(ePositionFirst);

      while (mpBktCurrent && bl.mpBktCurrent) {
        mpBktCurrent->mpLinkLast->mpLinkNext = bl.mpBktCurrent->mpLinkFirst;

        mpBktCurrent    = mpBktCurrent->mpBktNext;
        bl.mpBktCurrent = bl.mpBktCurrent->mpBktNext;
      }

      mBktDone = FALSE;
    }
    else {
      fprintf(stderr, "eWest or eNorth not supported\n");
    }

    // Now clear out 'bl', the actions should be same as constructor.
    // Danger: Do not call bl.Clean() as that will destroy the elements
    // that were just attached.
    bl.mpBktFirst    = bl.mpBktLast = 0;
    bl.mpLinkCurrent = 0;
    bl.mpBktCurrent  = 0;
    bl.mBucketTotal  = 0;
    bl.mBktDone      = TRUE;
    bl.mLinkDone     = TRUE;

#ifdef Caching
    // TBD: Also clear out cached stuff if caching is enabled.
    BktLink<T> *cachedLink;
    short cachedPos;
#endif
  }
  // else: nothing to do, bl is empty.
}

template<class T>
T* BucketList<T>::Get()
{
  if (mpLinkCurrent)
    return mpLinkCurrent->mpElem;

  return 0;
}

template<class T>
bool BucketList<T>::BktSetPosition(ePosition pos)
{
  switch (pos) {
    case ePositionFirst:
      mpBktCurrent = mpBktFirst;

      if (mpBktCurrent) {
        mpLinkCurrent = mpBktCurrent->mpLinkFirst;
        return TRUE;
      }
      else {
        mpLinkCurrent = 0;
        mBktDone      = TRUE;

        return FALSE;
      }
    case ePositionNext:
      if (mpBktCurrent) {
        mpBktCurrent = mpBktCurrent->mpBktNext;
        if (mpBktCurrent == 0) {
          mBktDone = TRUE;

          return FALSE;
        }

        return TRUE;
      }
      else {
        mBktDone = TRUE;

        return FALSE;
      }
    case ePositionLast:
      mpBktCurrent = mpBktLast;

      if (mpBktCurrent) {
        mpLinkCurrent = mpBktCurrent->mpLinkFirst;

        return TRUE;
      }
      else {
        mpLinkCurrent = 0;
        mBktDone      = TRUE;

        return FALSE;
      }
    case ePositionDone:
      mpBktCurrent = 0;
      mBktDone     = TRUE;

      return TRUE;
    case ePositionCurrent:
      return TRUE;
  }

  return FALSE;
}

template<class T>
ePosition BucketList<T>::BktGetPosition() const
{
  if      (mpBktCurrent == 0)          return ePositionDone;
  else if (mpBktCurrent == mpBktFirst) return ePositionFirst;
  else if (mpBktCurrent == mpBktLast)  return ePositionLast;
  else                                 return ePositionNext;
}

template<class T>
Bucket<T>* BucketList<T>::BktGet(ePosition pos)
{
  switch (pos) {
  case ePositionFirst:
    return mpBktFirst;
  case ePositionLast:
    return mpBktLast;
  case ePositionCurrent:
    return mpBktCurrent;
  case ePositionNext:
    if (mpBktCurrent) {
      Bucket<T> *bkt = mpBktCurrent;
      mpBktCurrent   = mpBktCurrent->mpBktNext;
      if (mpBktCurrent) mBktDone = TRUE; 

      return bkt;
    }
    else {
      return 0;
    }
  default:
    return 0;
  }
}

template<class T>
bool BucketList<T>::SetLinkPosition(ePosition pos)
{
  switch (pos) {
    case ePositionFirst:
      if (mpBktCurrent) {
        mpLinkCurrent = mpBktCurrent->mpLinkFirst;
        if (mpLinkCurrent == 0) mLinkDone = TRUE;

        return TRUE;
      }
      else {
        mLinkDone = TRUE;

        return FALSE;
      }

    case ePositionNext:
      if (mpLinkCurrent) {
        mpLinkCurrent = mpLinkCurrent->mpLinkNext;
        if (mpLinkCurrent == 0) mLinkDone = TRUE;

        return TRUE;
      }
      else {
        mLinkDone = TRUE;

        return FALSE;
      }

    case ePositionLast:
      if (mpBktCurrent) {
        mpLinkCurrent = mpBktCurrent->mpLinkLast;

        if (mpLinkCurrent == 0) {
          mLinkDone = TRUE;

          return FALSE;
        }
        else
          return TRUE;
      }
      else {
        mLinkDone = TRUE;

        return FALSE;
      }

    case ePositionDone:
      mpLinkCurrent = 0;
      mLinkDone     = TRUE;

      return TRUE;
  }
}

template<class T>
ePosition BucketList<T>::GetLinkPosition()
{
  if (mpBktCurrent) {
    if      (mpLinkCurrent == mpBktCurrent->mpLinkFirst) return ePositionFirst;
    else if (mpLinkCurrent == mpBktCurrent->mpLinkLast)  return ePositionLast;
    else if (mpLinkCurrent == 0)                         return ePositionDone;
    else                                                 return ePositionNext;
  }
  else
    return ePositionDone;
}

template<class T>
bool BucketList<T>::NextLink()
{
  if (mpLinkCurrent->mpLinkNext)
  {
    mpLinkCurrent = mpLinkCurrent->mpLinkNext;
    return TRUE;
  }
  else
    mpLinkCurrent = 0;

  return FALSE;
}

template<class T>
bool BucketList<T>::BktDone() const
{
  if (BktGetPosition() == ePositionDone) return TRUE;
  else                                   return FALSE;
}

template<class T>
void BucketList<T>::BktAppend(BucketList<T> &bl)
{
  if (mpBktCurrent) {
    // Make a copy of the bucket to be appended
    Bucket<T> bkt;
    bkt.CopyDeep(*bl.mpBktCurrent);

    if (mpBktCurrent->mpLinkLast) mpBktCurrent->mpLinkLast->mpLinkNext = bkt.mpLinkFirst;
    else                          mpBktCurrent->mpLinkLast             = bkt.mpLinkFirst;

    mpBktCurrent->mpLinkLast = bkt.mpLinkLast;

    bkt.Detach(); // prevent bkt destructor from deleting links/elem's appended
  }
  // else: this bucket list is empty or mpBktCurrent isnt correctly positioned
}

template<class T>
bool BucketList<T>::LinkDone()
{
  if (GetLinkPosition() == ePositionDone) return TRUE;
  else                                    return FALSE;
}


#endif

// End
