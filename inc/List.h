// $RCSfile: List.h,v $
// $Revision: 1.8 $

#ifndef LIST_CLASS
#define LIST_CLASS

// Template for a simple list of pointers. Classes that want
// to use this list need to define a copy constructor.

#include <stdio.h>
#include <assert.h>

#include "Global.h"

M_ExternConstructDestructCountInt;

#define MacroListDeleteAll(list, Type)                \
  for (list.First(); ! list.IsDone(); list.Next()) {  \
    Type *elm = list.Get();                           \
    assert(elm);                                      \
    delete elm;                                       \
  }                                                   \
  list.DeleteLinkAll();

#define MacroListDeleteLink(list, elm, ret)   \
  ret = list.PositionFind(elm);               \
  if (ret) { list.DeleteLinkThenNext(); }

#define MacroListDeleteElm(list, elm, ret)    \
  ret = list.PositionFind(elm);               \
  if (ret) { delete list.Get(); list.DeleteLinkThenNext(); }

#define MacroListIterateAll(list)             \
  (list).First(); ! (list).IsDone(); (list).Next()

#define MacroListIterateAllReverse(list)      \
  (list).Last(); ! (list).IsDone(); (list).Prev()

// Shortened macros, using only M_ prefix.
#define M_ListIterateAll(list)                \
  (list).First(); ! (list).IsDone(); (list).Next()

#define M_ListDeleteAll(list, type) MacroListDeleteAll(list, type)

#define M_ListIterateAndDelete(list, del_link)\
  (list).First(); ! (list).IsDone();          \
   (del_link) ? (list).DeleteLinkThenNext() : (list).Next()

enum eListLinkTransferType {
  eListLinkTransferType_FromCurrentToBeforeCurrent,
  eListLinkTransferType_FromCurrentToAfterCurrent
};

enum eBucket          { eFirstBucket, eLastBucket };
enum eAddDirection    { eSouth, eNorth, eEast, eWest };

enum eList_SplitPosition {
  eList_SplitBeforeIterator,
  eList_SplitAfterIterator
};

enum eListIteratorPolicyAfterDelete {
  eListIteratorPolicyAfterDelete_Prev,
  eListIteratorPolicyAfterDelete_Next,
  eListIteratorPolicyAfterDelete_Invalidate,
  eListIteratorPolicyAfterDelete_Assert0
};

enum eListAddPosition {
  eListAddPosition_First,
  eListAddPosition_BeforeCurrent,
  eListAddPosition_AfterCurrent,
  eListAddPosition_Last,
  eListAddPosition_NoChange
};

enum eListCurrentLinkAfterUnlink {
  eListCurrentLinkAfterUnlink_Prev,
  eListCurrentLinkAfterUnlink_Next,
  eListCurrentLinkAfterUnlink_Default
  // TBD: Add eListCurrentLinkAfterUnlink_Error
};

template<class T> class ListIterator;

#define eLinkAfterUnlink eListCurrentLinkAfterUnlink

template<class T>
class List
{
public:
  List();
  ~List();

  // Add elements to list (for simplicity and uniformity, use only pointers)
  bool  Add(T* elm, eListAddPosition add_position=eListAddPosition_Last, ListIterator<T>* iter=0);
  T*    Get(); // Retrieves the 'current' element

  // Re-link between lists and iterators.
  bool  RelinkTo(List<T>*           list, eListAddPosition pos, eLinkAfterUnlink next);
  bool  RelinkTo(ListIterator<T>*   iter, eListAddPosition pos, eLinkAfterUnlink next);
  //
  bool  RelinkFrom(List<T>*         list, eListAddPosition pos, eLinkAfterUnlink next);
  bool  RelinkFrom(ListIterator<T>* iter, eListAddPosition pos, eLinkAfterUnlink next);
  //
  bool  RelinkFromTo(ListIterator<T>* itr1, ListIterator<T>* itr2, eListAddPosition pos, eLinkAfterUnlink next);

  // Delete functions.
  void DeleteLinkAll      ();
  bool DeleteLinkThenNext ();
  bool DeleteLinkThenPrev ();
  bool DeleteLinkThenIterationDone();

  // Position the current element of the list, use this to iterate over list.
  bool First   ();        // Iteration can be accomplished as follows:
  bool Prev    ();        // for (First(); !IsDone(); Next())
  bool Next    ();        //   item_ptr = Get();
  bool Last    ();        // for (Last();  !IsDone() ; Prev())
  bool IsLast  () const;  //   item_ptr = Get();
  bool IsEmpty () const;
  bool IsDone  () const;

  bool ListIteratorDelete(ListIterator<T>* iter, bool delete_all=FALSE);

  // ListIterator.
  ListIterator<T>* ListIteratorNew();

  // Utilities:
  bool PositionFind    (const T* elm);
  bool PositionSet     (ListIterator<T>* iter);
  bool PositionSet     (unsigned position);
  bool PositionSetForIterator (ListIterator<T>* iter);
  bool PositionSave    (); // not allowed to save an illegal (0) position
  bool PositionRestore (); // false if saved position no longer exists
  void Split(List<T>& list, eList_SplitPosition pos);
  bool Unlink(eListCurrentLinkAfterUnlink pos=eListCurrentLinkAfterUnlink_Next, ListIterator<T>* iter=0);

  struct _Link {
    _Link() : next(0), prev(0), mElem(0) { M_UpdConstructCount(eType_Link); }
   ~_Link()                              { M_UpdDestructCount (eType_Link); }

    struct _Link* next;
    struct _Link* prev;
    T*            mElem;
  };
  typedef struct _Link Link;

  // Queries:
  bool     Exists(const T* elm) { return PositionFind(elm); }
  unsigned Size(void) const     { return mSize;             }
  Link*    CurrentLinkGet()     { return mpCurrentLink;     }
  bool     First(void*& link)   { link=mpFirst; return (link!=0) ? TRUE: FALSE; }
  bool     Last (void*& link)   { link=mpLast;  return (link!=0) ? TRUE: FALSE; }
  
  // Debug:
  short Print();

private:
  Link*     mpFirst;       // 0 only if list is empty
  Link*     mpLast;        //            "
  Link*     mpCurrentLink; //            "
  Link*     mpUnlinked;    //            "
  unsigned  mSize;         //            "
  bool      mAttemptedPrevForFirstElem;
  bool      mAttemptedNextForLastElem;
  bool      mIterationMarkedAsDone;
  //
  // Needed to save/restore the current links position.
  bool      mAttemptedPrevForFirstElemSaved;
  bool      mAttemptedNextForLastElemSaved;
  bool      mIterationMarkedAsDoneSaved;
  Link*     mpCurrentLinkSaved;
  //
  // Needed to support Iterators:
  short     mListIteratorCount;
  //
  #define LIST_MAX_ITERATORS  20
  ListIterator<T>** mpListIteratorList;
  
  // Private member functions.
  bool DeleteLinkCurrent(eListCurrentLinkAfterUnlink pos_after_unlink);
  bool StoreInFreeListIteratorEntry(ListIterator<T>* iter);
  void InformIteratorsOfUnlink(Link* deleted_link, eListCurrentLinkAfterUnlink pos_after_unlink, ListIterator<T>* iter);
  bool AddCommon(T* elm, eListAddPosition add_position, Link* new_link=0, ListIterator<T>* iter=0);
  bool Unlink(Link* unlink, eListCurrentLinkAfterUnlink pos_after_unlink, ListIterator<T>* iter=0);
};

template<class T>
List<T>::List() :
  mpLast                          (0),
  mpFirst                         (0),
  mAttemptedPrevForFirstElem      (FALSE),
  mAttemptedNextForLastElem       (FALSE),
  mIterationMarkedAsDone          (FALSE),
  mpCurrentLink                   (0),
  mpUnlinked                      (0),
  mSize                           (0),
  mListIteratorCount              (0),
  mpCurrentLinkSaved              (0),
  mAttemptedPrevForFirstElemSaved (0),
  mAttemptedNextForLastElemSaved  (0),
  mIterationMarkedAsDoneSaved     (0)
{
  mpListIteratorList = new ListIterator<T>* [LIST_MAX_ITERATORS];

  for (int i=0; i<LIST_MAX_ITERATORS; i++) mpListIteratorList[i] = 0;

  M_UpdConstructCount(eType_List);
}

template<class T>
List<T>::~List()
{
  // The list class does not know whether the pointers it stores are shared and
  // so it can only delete the links, the link elements need to be always
  // explicitly deleted by the client if necessary.
  DeleteLinkAll();

  delete [] mpListIteratorList; mpListIteratorList = 0;

  M_UpdDestructCount(eType_List);
}

template<class T>
bool List<T>::First()
{
  // Clear the 'attempt' flags.
  mAttemptedPrevForFirstElem = FALSE;
  mAttemptedNextForLastElem  = FALSE;
  mIterationMarkedAsDone     = FALSE;

  if (mpFirst) {
    mpCurrentLink = mpFirst;

    return TRUE;
  }

  return FALSE;
}

template<class T>
bool List<T>::Prev()
{
  if (!mpCurrentLink || mpCurrentLink==mpFirst) {
    mAttemptedPrevForFirstElem = TRUE;
    return FALSE;
  }
  else {
    mpCurrentLink = mpCurrentLink->prev;

    return TRUE;
  }

  return FALSE;
}

template<class T>
bool List<T>::Next()
{
  if (mpCurrentLink==0 || mpCurrentLink==mpLast) {
    mAttemptedNextForLastElem = TRUE;

    return FALSE;
  }
  else {
    mpCurrentLink = mpCurrentLink->next;
    return TRUE;
  }

  assert(0);
}

template<class T>
bool List<T>::Last()
{
  // Clear the 'attempt' flags.
  mAttemptedPrevForFirstElem = FALSE;
  mAttemptedNextForLastElem  = FALSE;

  if (Size() == 0) return FALSE;

  mpCurrentLink = mpLast;

  return TRUE;
}

template<class T>
bool List<T>::IsLast() const
{
  // Note: also returns true if list is empty.
  return (mpCurrentLink == mpLast);
}

template<class T>
bool List<T>::IsDone() const
{
  if (mSize==0 || mAttemptedPrevForFirstElem || mAttemptedNextForLastElem || mIterationMarkedAsDone)
    // Note: Also returning done if list is empty.
    return TRUE;
  else
    return FALSE;
}

template<class T>
bool List<T>::IsEmpty() const
{
  if (mSize) return FALSE;
  else       return TRUE;
}

template<class T>
short List<T>::Print()
{
  Link* link;
  short count=0;

  for (link=mpFirst; link!=0; link=link->next) {
    printf("%d: %x\n", count, (unsigned)link->mElem); // TBD: Fix for 64 bit
    count++;
  }

  return count;
}

template<class T>
T* List<T>::Get()
{
  if (mAttemptedNextForLastElem
  ||  mAttemptedPrevForFirstElem
  ||  mIterationMarkedAsDone
  || !mpCurrentLink)
  {
    return 0;
  }

  return mpCurrentLink->mElem;
}

template<class T>
bool List<T>::RelinkTo(List<T>* list, eListAddPosition pos_add, eLinkAfterUnlink pos_after_unlink)
{
  if (mpCurrentLink == 0) return FALSE; // no current link to unlink

  Link* unlink = mpCurrentLink;

  if (unlink == list->mpCurrentLink) {
    // Can happen when we are attempting to unlink from the same list.
    return TRUE;
  }
  else {
    if (Unlink(pos_after_unlink))
      list->AddCommon((T*)0, pos_add, unlink);
  }

  return TRUE;
}

template<class T>
bool List<T>::RelinkTo(ListIterator<T>* iter, eListAddPosition pos_add, eLinkAfterUnlink pos_after_unlink)
{
  // The  logic is almost identical to the RelinkTo(List<T>, ...) case.

  if (mpCurrentLink == 0) return FALSE; // no current link to unlink

  Link* unlink = mpCurrentLink;

  if (unlink == (Link*)(iter->mpCurrentLink)) {
    // Can happen when the iterator is from this list.
    return TRUE;
  }
  else {
    if (Unlink(pos_after_unlink))
      iter->mpList->AddCommon((T*)0, pos_add, unlink, iter);
  }

  return TRUE;
}

template<class T>
bool List<T>::RelinkFrom(List<T>* list, eListAddPosition pos_add, eLinkAfterUnlink pos_after_unlink)
{
  if (list->mpCurrentLink == 0) return FALSE; // no current link to unlink

  Link* unlink = list->mpCurrentLink;

  if (unlink == mpCurrentLink) {
    // Can happen when we are attempting to unlink from the same list.
    return TRUE;
  }
  else {
    if (list->Unlink(pos_after_unlink))
      AddCommon((T*)0, pos_add, unlink);
  }

  return TRUE;
}

template<class T>
bool List<T>::RelinkFrom(ListIterator<T>* iter, eListAddPosition pos_add, eLinkAfterUnlink pos_after_unlink)
{
  if (iter->mpCurrentLink == 0) return FALSE; // no current link to unlink

  Link* unlink = (Link*)(iter->mpCurrentLink);

  if (unlink == mpCurrentLink) {
    // Can happen when the iterator is from this list.
    return TRUE;
  }
  else {
    if (iter->mpList->Unlink(pos_after_unlink, iter))
      AddCommon((T*)0, pos_add, unlink);
  }

  return TRUE;
}

template<class T>
bool List<T>::RelinkFromTo(ListIterator<T>* itr1, ListIterator<T>* itr2, eListAddPosition pos_add, eLinkAfterUnlink pos_after_unlink)
{
  if (itr1->mpCurrentLink == 0) return FALSE; // no current link to unlink

  Link* unlink = (Link*)(itr1->mpCurrentLink);
  Link* relink = (Link*)(itr2->mpCurrentLink);

  if (unlink == relink) {
    return TRUE;
  }
  else {
    if (itr1->mpList->Unlink(pos_after_unlink, itr1))
      AddCommon((T*)0, pos_add, unlink, itr2);
  }

  return TRUE;
}

template<class T>
bool List<T>::Add(T* elm, eListAddPosition add_position, ListIterator<T>* iter)
{
  return AddCommon(elm, add_position, (Link*)0, iter);
}

template<class T>
bool List<T>::AddCommon(T* elm, eListAddPosition add_position, Link* new_link, ListIterator<T>* iter)
{
  if (!new_link) {
    new_link = new Link;
    assert(new_link);

    new_link->mElem = elm;
  }

  new_link->next  = 0;

  Link* curr_link_saved;

  // If the 'Add' is for an iterator then save the current link,
  // it will be restored after the Add operation.
  if (iter) {
    curr_link_saved = mpCurrentLink;
    mpCurrentLink   = (Link*)(iter->mpCurrentLink);
  }
  // All other logic for the 'add' operation (below) remains the same.

  switch (add_position) {
    case eListAddPosition_First:
    {
      new_link->prev = 0;

      if (mpFirst) {
        new_link->next = mpFirst;
        //
        mpFirst->prev = new_link;
        mpFirst = new_link;
      }
      else {
        mpFirst = mpLast = new_link;
        new_link->next = 0;
      }
    }
    break;
    case eListAddPosition_BeforeCurrent:
    {
      if (mpCurrentLink==0) {
        mpFirst = mpLast = mpCurrentLink = new_link;
        new_link->prev = 0;
      }
      else if (mpCurrentLink==mpFirst && mpCurrentLink==mpLast) {
        new_link->prev = 0;
        new_link->next = mpLast;
        //
        mpFirst      = new_link;
        mpLast->prev = new_link;
      }
      else if (mpCurrentLink==mpFirst) {
        new_link->prev = 0;
        new_link->next = mpFirst;
        //
        mpFirst->prev = new_link;
        mpFirst       = new_link;
      }
      else if (mpCurrentLink==mpLast) {
        mpLast->prev->next = new_link;
        //
        new_link->prev = mpLast->prev;
        new_link->next = mpLast;
        //
        mpLast->prev = new_link;
      }
      else {
        new_link->prev = mpCurrentLink->prev;
        new_link->next = mpCurrentLink;
        //
        mpCurrentLink->prev->next = new_link;
        mpCurrentLink->prev       = new_link;
      }
      break;
    }
    break;
    case eListAddPosition_AfterCurrent:
    {
      if (mpCurrentLink==0) {
        mpFirst = mpLast = mpCurrentLink = new_link;
        new_link->prev = 0;
      }
      else if (mpCurrentLink==mpFirst && mpCurrentLink==mpLast) {
        new_link->prev = mpFirst;
        //
        mpFirst->next  = new_link;
        mpLast         = new_link;
      }
      else if (mpCurrentLink==mpFirst) {
        new_link->prev = mpFirst;
        new_link->next = mpFirst->next;
        //
        mpFirst->next->prev = new_link;
        mpFirst->next       = new_link;
      }
      else if (mpCurrentLink==mpLast) {
        new_link->prev = mpLast;
        mpLast->next   = new_link;
        mpLast         = new_link;
      }
      else {
        new_link->prev = mpCurrentLink;
        new_link->next = mpCurrentLink->next;
        //
        mpCurrentLink->next->prev = new_link;
        mpCurrentLink->next       = new_link;
      }
      break;
    }
    break;
    case eListAddPosition_Last:
    {
      if (mpLast) {
        new_link->prev = mpLast;
        //
        mpLast->next   = new_link;
        mpLast         = new_link;
      }
      else {
        mpFirst = mpLast = new_link;
        new_link->prev = 0;
      }
    }
    break;
  }

  mSize++;

  // If we added to the end of the list then we can no longer claim
  // to have attempted 'next' for the last element of the list.
  if (new_link == mpLast) mAttemptedNextForLastElem = FALSE;

  // Similarly, if we added to the start of the list then we can no longer
  // claim to have attempted 'prev' for the first element of the list.
  if (new_link == mpFirst) mAttemptedPrevForFirstElem = FALSE;

  if (iter) {
    // Restore the lists current link unless it had no entries
    // (ie mpCurrentLink=0), in that case we cannot allow
    // mpCurrentLink to remain at 0 as that implies an empty list.

    if (curr_link_saved) mpCurrentLink = curr_link_saved;
    else                 mpCurrentLink = new_link;
  }

  return TRUE;
}

// Detach the current link from the list without deleting it.
template<class T>
bool List<T>::Unlink(eLinkAfterUnlink pos_after_unlink, ListIterator<T>* iter)
{
  if (iter == 0) return Unlink(mpCurrentLink,                pos_after_unlink);
  else           return Unlink((Link*)(iter->mpCurrentLink), pos_after_unlink, iter);
}

template<class T>
bool List<T>::Unlink(Link* unlink, eLinkAfterUnlink pos_after_unlink, ListIterator<T>* iter)
{
  if (mpCurrentLink == 0) return FALSE; // no entries in the list

  // An iterator is not allowed to unlink a lists current link, check.
  if (iter!=0 && unlink==mpCurrentLink)
    assert(0);

  InformIteratorsOfUnlink(unlink, pos_after_unlink, iter);

  if (unlink==mpFirst && unlink==mpLast) {
    mpCurrentLink = mpFirst = mpLast = 0;

    assert(iter==0);
  }
  else if (unlink == mpFirst) {
    // No need to update mpLast.
    // Update mpFirst (and unlink).
    mpFirst       = mpFirst->next;
    mpFirst->prev = 0;

    // Update mpCurrent.
    if (unlink == mpCurrentLink) {
      if (pos_after_unlink == eListCurrentLinkAfterUnlink_Next) {
        mpCurrentLink = mpFirst;
      }
      else if (pos_after_unlink == eListCurrentLinkAfterUnlink_Prev) {
        mpCurrentLink = mpFirst; // cannot go to prev

        mAttemptedPrevForFirstElem = TRUE;
      }
      else assert(0);
    }
  }
  else if (unlink == mpLast) {
    // No need to update mpFirst.
    // Update mpLast (this also unlinks).
    mpLast       = mpLast->prev;
    mpLast->next = 0;

    // Update mpCurrent.
    if (unlink == mpCurrentLink) {
      if (pos_after_unlink == eListCurrentLinkAfterUnlink_Next) {
        mpCurrentLink = mpLast; // cannot really go to next

        mAttemptedNextForLastElem = TRUE;
      }
      else if (pos_after_unlink == eListCurrentLinkAfterUnlink_Prev) {
        mpCurrentLink       = mpCurrentLink->prev;
        mpCurrentLink->next = 0;
      }
      else assert(0);
    }
  }
  else {
    // No need to update mpFirst.
    // No need to update mpLast.
    // Update mpCurrentLink (this also unlinks).

    if (unlink == mpCurrentLink) {
      if (pos_after_unlink == eListCurrentLinkAfterUnlink_Next)
        mpCurrentLink = mpCurrentLink->next;
      else if (pos_after_unlink == eListCurrentLinkAfterUnlink_Prev)
        mpCurrentLink = mpCurrentLink->prev;
      else assert(0);
    }

    unlink->prev->next = unlink->next;
    unlink->next->prev = unlink->prev;
  }

  mSize--;

  return TRUE;
}

template<class T>
bool List<T>::DeleteLinkThenIterationDone()
{
  // After delete the current link position is a 'dont-care'.
  mIterationMarkedAsDone = TRUE;

  return DeleteLinkCurrent(eListCurrentLinkAfterUnlink_Next);
}

template<class T>
bool List<T>::DeleteLinkThenNext()
{
  return DeleteLinkCurrent(eListCurrentLinkAfterUnlink_Next);
}

template<class T>
bool List<T>::DeleteLinkThenPrev()
{
  return DeleteLinkCurrent(eListCurrentLinkAfterUnlink_Prev);
}

// This is a private member function called only by
//   DeleteLinkThenNext(), and
//   DeleteLinkThenPrev()
template<class T>
bool List<T>::DeleteLinkCurrent(eListCurrentLinkAfterUnlink pos_after_unlink)
{
  if (!mpCurrentLink) return FALSE;

  Link* link_to_delete = mpCurrentLink;

  bool flag = Unlink(mpCurrentLink, pos_after_unlink);
  assert(flag);

  // After Unlink the value of mpCurrentLink would have been updated,
  // therefore, we cannot do 'delete mpCurrentLink;'
  delete link_to_delete;

  return flag;
}

template <class T>
void List<T>::DeleteLinkAll()
{
  mpCurrentLink = mpFirst;

  while (mpFirst) DeleteLinkThenNext();
}

// Tries to find elm and set the Iterator at that
// position otherwise it leaves the Iterator unchanged.
template<class T>
bool List<T>::PositionFind(const T* elm)
{
  Link* link;
  bool  found = FALSE;

  for (link=mpFirst; link != 0; link=link->next) {
    if (link->mElem == elm) { found = TRUE; break; }
  }

  if (found) {
    mpCurrentLink = link;

    // Clear the 'Attempt' flags.
    mAttemptedPrevForFirstElem = FALSE;
    mAttemptedNextForLastElem  = FALSE;
    mIterationMarkedAsDone     = FALSE;
  }

  return found;
}

template<class T>
bool List<T>::PositionSet(ListIterator<T>* iter)
{
  Link* ptr;
  // TBD: Check if iter is valid.

  // PREV:
  //void* ptr = (void*)iter->mpCurrentLink;
  //iter->mpCurrentLink  = (ListIterator::Link*)mpCurrentLink;
  if ((ptr = (Link*)iter->mpCurrentLink)==0 && mSize!=0) {
    // An iterator is not allowed to set the position of the current
    // link 'mpCurrentLink' to 0 if the list has entries in it.
    return FALSE;
  }
  else {
    mpCurrentLink      = (Link*)iter->mpCurrentLink;
    mpCurrentLinkSaved = 0; // safer/cleaner to not copy over the Saved position

    mAttemptedPrevForFirstElem = iter->mAttemptedPrevForFirstElem;
    mAttemptedNextForLastElem  = iter->mAttemptedNextForLastElem;
    mIterationMarkedAsDone     = iter->mIterationMarkedAsDone;
  }
}

// Tries to find elm at 'position' and sets the Iterator to
// that position otherwise it leaves the Iterator unchanged.
template<class T>
bool List<T>::PositionSet(unsigned position)
{
  Link*    link;
  bool     found = FALSE;
  unsigned count = 0;

  for (link=mpFirst; link!=0; link=link->next) {
    if (position == count) { found = TRUE; break; }
    else                   { count++;             }
  }

  if (found) {
    mpCurrentLink = link;

    // Clear the 'Attempt' flags.
    mAttemptedPrevForFirstElem = FALSE;
    mAttemptedNextForLastElem  = FALSE;
    mIterationMarkedAsDone     = FALSE;
  }

  return found;
}

template <class T>
bool List<T>::PositionSetForIterator(ListIterator<T>* iter)
{
  iter->mAttemptedPrevForFirstElem = mAttemptedPrevForFirstElem;
  iter->mAttemptedNextForLastElem  = mAttemptedNextForLastElem;
  iter->mIterationMarkedAsDone     = mIterationMarkedAsDone;
  iter->CurrentLinkSet((void*)mpCurrentLink);

  return TRUE;
}

// Save the Iterator position (Iterator must be non-null).
template<class T>
bool List<T>::PositionSave()
{
  if (mpCurrentLink) {
    mpCurrentLinkSaved              = mpCurrentLink;
    mAttemptedPrevForFirstElemSaved = mAttemptedPrevForFirstElem;
    mAttemptedNextForLastElemSaved  = mAttemptedNextForLastElem;
    mIterationMarkedAsDoneSaved     = mIterationMarkedAsDone;

    return TRUE;
  }

  // Not allowed to save the position when mpCurrentLink is 0.
  return FALSE;
}

// Restore the Iterator position.
// After Restore the saved position is invalidated, therefore user must call
// Restore again if the Saved position is still needed.
// The Restore fails if:
// a) Saved was not first called
// b) Saved position was deleted.
template<class T>
bool List<T>::PositionRestore()
{
  // Not allowed to Restore a position if it has not been saved.
  if (!mpCurrentLinkSaved) return FALSE;

  mpCurrentLink = mpCurrentLinkSaved;

  mpCurrentLinkSaved         = 0;
  mAttemptedPrevForFirstElem = mAttemptedPrevForFirstElemSaved;
  mAttemptedNextForLastElem  = mAttemptedNextForLastElemSaved;
  mIterationMarkedAsDone     = mIterationMarkedAsDoneSaved;

  return TRUE;
}

// Split 'this' list at mpCurrentLink such that mpCurrentLink is either
// the last link in the current list or first link of the passed in 'list'.
template<class T>
void List<T>::Split(List<T>& list, eList_SplitPosition pos)
{
  if (pos == eList_SplitBeforeIterator) {
    assert(0); // not yet supported
  }

  // Subsequent logic will split after link mpCurrentLink.

  if (mpCurrentLink == 0)
  {
    // Current list is either empty or iterator is past the first/last element.
    return;
  }

  // if 'list.mpLast' is set then the passed in list already has entries, check:
  if (list.mpLast) list.mpLast->next = mpCurrentLink->next; // had entries, append.
  else             list.mpFirst      = mpCurrentLink->next; // List is empty.

  list.mSize += mSize - 1;
  mSize = 1;

  list.mpLast      = mpLast;

  // In both above cases we can leave the following untouched in each list:
  //   mAttemptedPrevForFirstElem
  //   mAttemptedNextForLastElem
  //   mpCurrentLinkSaved
  //   mListIteratorCount

  // Truncate current list to mpCurrentLink.
  mpLast           = mpCurrentLink;
  mpLast->next     = 0;

  // Invalidate 'Saved' links.
  mpCurrentLinkSaved     = 0; // otherwise re-scan to ensure its valid
}

template <class T>
ListIterator<T>* List<T>::ListIteratorNew()
{
  ListIterator<T>* iter;

  if (mListIteratorCount+1 < LIST_MAX_ITERATORS) {
    iter = new ListIterator<T>(*this);
    assert(iter);

    assert (StoreInFreeListIteratorEntry(iter));

    mListIteratorCount++;
  }

  return iter;
}

template <class T>
bool List<T>::ListIteratorDelete(ListIterator<T>* iter, bool delete_all)
{
  int last_entry = mListIteratorCount-1;

  if (delete_all) {
    for (int i=0, j=0; i<LIST_MAX_ITERATORS && j<mListIteratorCount; i++) {
      if (mpListIteratorList[i] != 0) {
        delete mpListIteratorList[i];
        mpListIteratorList[i] = 0;
        j++;
      }
    }

    return TRUE;
  }
  else {
    for (int i=0; i<mListIteratorCount; i++) {
      if (mpListIteratorList[i] == iter) {
        delete mpListIteratorList[i];

        if (i != last_entry) {
          mpListIteratorList[i] = mpListIteratorList[last_entry];
        }
        mpListIteratorList[last_entry] = 0;
        mListIteratorCount--;

        return TRUE;
      }
    }
  }

  return FALSE;
}

template <class T>
bool List<T>::StoreInFreeListIteratorEntry(ListIterator<T>* iter)
{
  for (int i=0; i<LIST_MAX_ITERATORS; i++) {
    if (mpListIteratorList[i] == 0) {
      mpListIteratorList[i] = iter;

      return TRUE;
    }
  }

  assert(0);

  return FALSE;
}

template <class T>
void List<T>::InformIteratorsOfUnlink(Link* deleted_link, eListCurrentLinkAfterUnlink pos_after_unlink, ListIterator<T>* iter)
{
  for (int i=0; mListIteratorCount && i<LIST_MAX_ITERATORS; i++) {
    if (mpListIteratorList[i] != 0) {
      if (mpListIteratorList[i] != iter) mpListIteratorList[i]->InformOfUnlink(deleted_link);
      else                               mpListIteratorList[i]->InformOfUnlink(deleted_link, pos_after_unlink);
    }
  }
}

template<class T>
class ListIterator
{
  friend class List<T>;

public:
  ListIterator(List<T> & list);
  ~ListIterator();

  bool First();
  bool Prev();
  bool Next();
  bool Last();
  bool IsLast()  const;
  bool IsEmpty() const;
  bool IsDone()  const;

  T*   Get();
  bool PositionSet(List<T>* list);
  bool PositionSet(ListIterator<T>* iter);
  bool PositionSave();
  bool PositionRestore();

  struct _Link {
    struct _Link* next;
    struct _Link* prev;
    T*            mElem;
  };
  typedef struct _Link Link;

  void InformOfUnlink(void* unlink, eListCurrentLinkAfterUnlink pos_after_unlink=eListCurrentLinkAfterUnlink_Default);
  Link* CurrentLinkGet() { return mpCurrentLink; }
  void  CurrentLinkSet(void* link) { mpCurrentLink = (Link*)link; }

  eListIteratorPolicyAfterDelete mListIteratorDeletePolicy;

private:
  List<T>*  mpList;
  Link*     mpCurrentLink;
  bool      mAttemptedNextForLastElem;
  bool      mAttemptedPrevForFirstElem;
  bool      mIterationMarkedAsDone;
  bool      mIteratorLinkValid;
  //
  Link*     mpCurrentLinkSaved;
  bool      mAttemptedNextForLastElemSaved;
  bool      mAttemptedPrevForFirstElemSaved;
  bool      mIterationMarkedAsDoneSaved;
};

template<class T>
ListIterator<T>::ListIterator(List<T>& list) :
  mpList                          (&list),
  mpCurrentLink                   ((Link*)list.CurrentLinkGet()),
  mAttemptedNextForLastElem       (FALSE),
  mAttemptedPrevForFirstElem      (FALSE),
  mIterationMarkedAsDone          (FALSE),
  mIteratorLinkValid              (TRUE),
  //
  mpCurrentLinkSaved              (0),
  mAttemptedNextForLastElemSaved  (FALSE),
  mAttemptedPrevForFirstElemSaved (FALSE),
  mIterationMarkedAsDoneSaved     (FALSE),
  //
  mListIteratorDeletePolicy       (eListIteratorPolicyAfterDelete_Invalidate)
{
  M_UpdConstructCount(eType_ListIterator);
}

template<class T>
ListIterator<T>::~ListIterator()
{
  M_UpdDestructCount(eType_ListIterator);
}

template<class T>
T* ListIterator<T>::Get()
{
  if (mAttemptedNextForLastElem
  ||  mAttemptedPrevForFirstElem
  ||  mIterationMarkedAsDone
  || !mpCurrentLink)
  {
    return 0;
  }

  return mpCurrentLink->mElem;
}

template<class T>
bool ListIterator<T>::First()
{
  mAttemptedNextForLastElem  = FALSE;
  mAttemptedPrevForFirstElem = FALSE;

  void* ptr;
  bool ret = mpList->First(ptr);

  mpCurrentLink = (Link*)ptr;

  return ret;
}

template<class T>
bool ListIterator<T>::Prev()
{
  if (!mpCurrentLink || mpCurrentLink->prev == 0) 
  {
    mAttemptedPrevForFirstElem = TRUE;

    return FALSE;
  }
  else {
    mpCurrentLink = mpCurrentLink->prev;
    
    return TRUE;
  }

  assert(0); // should not reach here
}

template<class T>
bool ListIterator<T>::Next()
{
  if (mpCurrentLink==0 || mpCurrentLink->next == 0) 
  {
    mAttemptedNextForLastElem = TRUE;

    return FALSE;
  }
  else {
    mpCurrentLink = mpCurrentLink->next;

    return TRUE;
  }

  assert(0); // should not reach here
}

template<class T>
bool ListIterator<T>::Last()
{
  mAttemptedNextForLastElem  = FALSE;
  mAttemptedPrevForFirstElem = FALSE;

  void* ptr;
  bool ret = mpList->Last(ptr);

  mpCurrentLink = (Link*)ptr;

  return ret;
}

template<class T>
bool ListIterator<T>::IsLast() const
{
  Link* last;
  void* ptr;

  mpList->Last(ptr);

  last = (Link*)ptr;

  if (mpCurrentLink == last) return TRUE;
  else                       return FALSE;
}

template<class T>
bool ListIterator<T>::IsEmpty() const
{
  return (mpList->Size() == 0);
}

template<class T>
bool ListIterator<T>::IsDone() const
{
  return (IsEmpty() || mAttemptedPrevForFirstElem || mAttemptedNextForLastElem || mIterationMarkedAsDone);
}

template<class T>
bool ListIterator<T>::PositionSet(List<T>* list)
{
  return list->PositionSetForIterator(this);
}

template<class T>
bool ListIterator<T>::PositionSet(ListIterator<T>* iter)
{
  if (!iter->mIteratorLinkValid) return FALSE;

  mpCurrentLink              = iter->mpCurrentLink;
  mAttemptedNextForLastElem  = iter->mAttemptedNextForLastElemSaved;
  mAttemptedPrevForFirstElem = iter->mAttemptedPrevForFirstElemSaved;
  mIterationMarkedAsDone     = iter->mIterationMarkedAsDoneSaved;
}

template<class T>
bool ListIterator<T>::PositionSave()
{
  // The current link must be valid to save its position.
  // Also, we use 0 as an invalid mpCurrentLinkSaved.
  if (mpCurrentLink == 0) return FALSE;

  if (!mIteratorLinkValid) return FALSE;

  mpCurrentLinkSaved              = mpCurrentLink;
  mAttemptedNextForLastElemSaved  = mAttemptedNextForLastElem;
  mAttemptedPrevForFirstElemSaved = mAttemptedPrevForFirstElem;
  mIterationMarkedAsDoneSaved     = mIterationMarkedAsDone;

  return TRUE;
}

template<class T>
bool ListIterator<T>::PositionRestore()
{
  //  Check if restore position is valid.
  if (mpCurrentLinkSaved != 0) {
    mpCurrentLink              = mpCurrentLinkSaved;
    mAttemptedNextForLastElem  = mAttemptedNextForLastElemSaved;
    mAttemptedPrevForFirstElem = mAttemptedPrevForFirstElemSaved;
    mIterationMarkedAsDone     = mIterationMarkedAsDoneSaved;

    mpCurrentLinkSaved              = 0;
    mAttemptedNextForLastElemSaved  = 0;
    mAttemptedPrevForFirstElemSaved = 0;
  }
}

template <class T>
void ListIterator<T>::InformOfUnlink(void* unlink, eListCurrentLinkAfterUnlink pos_after_unlink)
{
  if ((void*)mpCurrentLink == unlink)
  {
    if (pos_after_unlink == eListCurrentLinkAfterUnlink_Default) {
      switch (mListIteratorDeletePolicy) {
        case eListIteratorPolicyAfterDelete_Prev:       if (!Prev()) Next();        break;
        case eListIteratorPolicyAfterDelete_Next:       if (!Next()) Prev();        break;
        case eListIteratorPolicyAfterDelete_Invalidate: mIteratorLinkValid = FALSE; break;
        case eListIteratorPolicyAfterDelete_Assert0:    assert(0);
        default:                                        assert(0);
      }
    }
    else {
      switch (pos_after_unlink) {
        case eListCurrentLinkAfterUnlink_Next: if (!Next()) Prev(); break;
        case eListCurrentLinkAfterUnlink_Prev: if (!Prev()) Next(); break;
        default:                               assert(0);
      }
    }
  }
}

#endif

// End
