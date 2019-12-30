#ifndef VIEWJOIN
#define VIEWJOIN

#include "List.h"
#include "Record.h"

class Query;
class View;

class ViewJoin
{
public:
  ViewJoin(Query* q);
 ~ViewJoin();

  bool RecordJoinSearchFirst();
  bool RecordJoinSearchNext();
  bool RecordJoinSearchIsDone();

  Record* RecordGet(View* view);

private:
  List<ViewIterator> mViewIteratorList;
  bool               mJoinSearchIsDone;
  Query*             mpQuery;
};

#endif
