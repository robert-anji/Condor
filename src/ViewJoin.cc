// $RCSfile: ViewJoin.cc,v $
// $Revision: 1.1 $

#include "ViewJoin.h"

#include "BucketList.h"
#include "Query.h"
#include "Record.h"

ViewJoin::ViewJoin(Query* q) :
  mJoinSearchIsDone(FALSE),
  mpQuery(0)
{
  assert(mpQuery);

  mpQuery = new Query(*q);

  BucketList<Condition> & cbl = q->mConditionBktList;

  assert(cbl.BktSetPosition(ePositionFirst));

  // In order to traverse all the records from each view in
  // the 'join' we allocate a ViewIterator for each unique View
  // that was stored within the Attributes of the Condition's
  // when the query was formed.
  for (cbl.BktSetPosition(ePositionFirst); !cbl.BktDone(); cbl.BktSetPosition(ePositionNext)) {
    for (cbl.SetLinkPosition(ePositionFirst); !cbl.LinkDone(); cbl.SetLinkPosition(ePositionNext))
    {
      Condition* c = cbl.Get();

      const Attribute* attr;

      for (int i=0; i<2; i++) {
        if (i==0) attr = c->mpAttributeLHS; // the logic below is needed for both
        if (i==1) attr = c->mpAttributeRHS; // mpAttributeLHS and mpAttributeRHS.

        if (attr) {
          assert(attr->mpView); // for Joins the Condition attributes must specify a view

          // Check if a ViewIterator has been allocated already for this view.
          bool found = FALSE;
          ViewIterator* vi;

          for (MacroListIterateAll(mViewIteratorList)) {
            ViewIterator* vi = mViewIteratorList.Get();
            assert(vi);

            if (vi->mpView == attr->mpView) { found=TRUE; break; }
          }

          if (!found) {
            vi = attr->mpView->ViewIteratorNew();
            mViewIteratorList.Add(vi);
          }

          if (i==0) c->mpViewIteratorLHS = vi;
          if (i==1) c->mpViewIteratorRHS = vi;
        }
      }
    }
  }
}

ViewJoin::~ViewJoin()
{
  for (MacroListIterateAll(mViewIteratorList)) {
    ViewIterator* vi = mViewIteratorList.Get();
    assert(vi);

    delete vi;
  }

  delete mpQuery;
}

bool ViewJoin::RecordJoinSearchFirst()
{
  mJoinSearchIsDone = FALSE;

  // 1. Position all View Iterators to the First position.
  for (MacroListIterateAll(mViewIteratorList)) {
    ViewIterator* vi = mViewIteratorList.Get();
    assert(vi);

    assert (vi->RecordSearchFirst()); // for initial algo we dont allow empty views in join
  }

  if (mpQuery->Match()) return TRUE;

  return RecordJoinSearchNext();
}

bool ViewJoin::RecordJoinSearchNext()
{
  // 1. Position to the last ViewIterator for the (innermost) loop.
  mViewIteratorList.Last();
  ViewIterator* vi = mViewIteratorList.Get();
  assert(vi);

  bool more_outer_records = TRUE;
  bool found              = FALSE;

  // 2. Repeat the innermost loop as long as there are outer ViewIterator records.
  //    The first time outer records are guaranteed because of step 1.
  while (more_outer_records) {
    for (vi->RecordSearchNext(); ! vi->RecordSearchIsDone(); vi->RecordSearchNext())
    {
      if (mpQuery->Match()) { found=TRUE; break; }
    }
    vi->RecordSearchFirst(); // in preparation of next loop

    // No match found in innermost loop, try to advance one of the outer ViewIterators.
    more_outer_records = FALSE;
    while (mViewIteratorList.Prev()) {
      ViewIterator* vi = mViewIteratorList.Get();
      assert(vi);

      if (vi->RecordSearchNext()) { more_outer_records=TRUE; break; }
      else                        { vi->RecordSearchFirst(); } // in preparation of next loop
    }
  }

  if (! found) mJoinSearchIsDone=TRUE;

  return found;
}

bool ViewJoin::RecordJoinSearchIsDone()
{
  return mJoinSearchIsDone;
}

Record* ViewJoin::RecordGet(View* view)
{
  ViewIterator* vi;

  for (MacroListIterateAll(mViewIteratorList)) {
    vi = mViewIteratorList.Get();
    assert(vi);

    if (vi->mpView == view) return vi->RecordCurrentGet();
  }

  return 0;
}

// End
